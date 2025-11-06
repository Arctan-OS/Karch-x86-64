/**
 * @file context.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Karch-x86-64 - x86-64 Implementation of K/arch Abstractions
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Karch-x86-64.
 *
 * Arctan-OS/Karch-x86-64 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include "arch/context.h"
#include "arch/smp.h"
#include "arch/x86-64/context.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/smp.h"
#include "arch/x86-64/sse.h"
#include "arctan.h"
#include "global.h"
#include "mm/allocator.h"
#include "util.h"

#include <cpuid.h>

#define FS_BASE_MSR  0xC0000100
#define GS_BASE_MSR  0xC0000101
#define KGS_BASE_MSR 0xC0000102

void context_set_tcb(ARC_Context *ctx, void *tcb) {
        _x86_WRMSR(FS_BASE_MSR, (uintptr_t)tcb);
        ctx->tcb = tcb;
}

void *context_get_tcb(ARC_Context *ctx) {
        return ctx->tcb;
}

void context_set_proc_desc(ARC_x64ProcessorDescriptor *desc) {
        _x86_WRMSR(GS_BASE_MSR, (uintptr_t)desc);
}

USERSPACE(text) ARC_x64ProcessorDescriptor *context_get_proc_desc() {
        return (ARC_x64ProcessorDescriptor *)_x86_RDMSR(GS_BASE_MSR);
}

void context_save(ARC_Context *ctx, ARC_InterruptFrame *new) {
        memcpy(&ctx->frame, new, sizeof(*new));
        // NOTE: TCB does not need to be saved here, that is done in
        //       context_set_tcb
        if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_XSAVE)) {
                __asm__("xor rax, rax; \
                         dec rax;                                       \
                         mov rdx, rax;                                  \
                         xsave [%0];" :: "r"(ctx->xsave_space) : "rax", "rdx");
        } else if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_FXSAVE)){
                 __asm__("xor rax, rax; \
                         dec rax;                                       \
                         mov rdx, rax;                                  \
                         fxsave [%0];" :: "r"(ctx->xsave_space) : "rax", "rdx");
        }
}

void context_load(ARC_Context *ctx, ARC_InterruptFrame *to) {
        memcpy(to, &ctx->frame, sizeof(*to));
        _x86_WRMSR(FS_BASE_MSR, (uintptr_t)ctx->tcb);

        if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_XSAVE)) {
                __asm__("xor rax, rax; \
                         dec rax;                                       \
                         mov rdx, rax;                                  \
                         xrstor [%0];" :: "r"(ctx->xsave_space) : "rax", "rdx");
        } else if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_FXSAVE)){
                 __asm__("xor rax, rax; \
                         dec rax;                                       \
                         mov rdx, rax;                                  \
                         fxrstor [%0];" :: "r"(ctx->xsave_space) : "rax", "rdx");
        }

}

int uninit_context(ARC_Context *context) {
        if (context == NULL) {
                return -1;
        }

        free(context->xsave_space);
        free(context);

        return 0;
}

int context_set_proc_features(ARC_ProcessorFeatures *features) {
        register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

        __cpuid(0x01, eax, ebx, ecx, edx);

        if (MASKED_READ(edx, 24, 1)) {
                // This should also enable
                features->proc0 |= 1 << ARC_PROC0_FLAG_FXSAVE;
                uint64_t cr4 = _x86_getCR4() | (1 << 9); // OSFXSR
                _x86_setCR4(cr4);
        } else {
                ARC_DEBUG(ERR, "No fxsave/rstor instructions\n");
        }

        if (MASKED_READ(ecx, 26, 1)) {
                features->proc0 |= 1 << ARC_PROC0_FLAG_XSAVE;
                uint64_t cr4 = _x86_getCR4() | (1 << 18); // OSXSAVE
                _x86_setCR4(cr4);
        } else {
                ARC_DEBUG(ERR, "No xsave/rstor instructions\n");
        }

        if (MASKED_READ(ecx, 17, 1)) {
                features->paging |= 1 << ARC_PAGER_FLAG_PCID;
                uint64_t cr4 = _x86_getCR4() | 1 << 17; // Set PCIDE bit (17)
                _x86_setCR4(cr4);
                ARC_DEBUG(INFO, "PCIDs supported\n");
        }

        features->proc0 |= MASKED_READ(ecx, 25, 1) << ARC_PROC0_FLAG_SSE1;
        features->proc0 |= MASKED_READ(ecx, 26, 1) << ARC_PROC0_FLAG_SSE2;
        features->proc0 |= MASKED_READ(edx, 1,  1) << ARC_PROC0_FLAG_SSE3;
        features->proc0 |= MASKED_READ(edx, 9,  1) << ARC_PROC0_FLAG_SSSE3;
        features->proc0 |= MASKED_READ(edx, 19, 1) << ARC_PROC0_FLAG_SSE4_1;
        features->proc0 |= MASKED_READ(edx, 20, 1) << ARC_PROC0_FLAG_SSE4_2;

        features->proc0 |= MASKED_READ(ecx, 4,  1) << ARC_PROC0_FLAG_TSC;
        features->proc0 |= MASKED_READ(ecx, 9,  1) << ARC_PROC0_FLAG_APIC;
        features->proc0 |= MASKED_READ(ecx, 5,  1) << ARC_PROC0_FLAG_MSR;
        features->proc0 |= MASKED_READ(ecx, 19, 1) << ARC_PROC0_FLAG_CLFLUSH;
        features->proc0 |= MASKED_READ(ecx, 27, 1) << ARC_PROC0_FLAG_SELF_SNOOP;

        features->proc0 |= MASKED_READ(edx, 5,  1) << ARC_PROC0_FLAG_VMX;
        features->proc0 |= MASKED_READ(edx, 21, 1) << ARC_PROC0_FLAG_X2APIC;
        features->proc0 |= MASKED_READ(edx, 31, 1) << ARC_PROC0_FLAG_HYPERVISOR;
        features->proc0 |= MASKED_READ(edx, 28, 1) << ARC_PROC0_FLAG_AVX;
        features->proc0 |= MASKED_READ(edx, 30, 1) << ARC_PROC0_FLAG_RDRND;

        __cpuid(0x7, eax, ebx, ecx, edx);

        if (MASKED_READ(ecx, 16, 1)) {
                ARC_DEBUG(INFO, "PML5 supported\n");
                features->paging |= 1 << ARC_PAGER_FLAG_PML5;
        }

        if (MASKED_READ(ecx, 31, 1)) {
                ARC_DEBUG(INFO, "PKS supported\n");
                features->paging |= 1 << ARC_PAGER_FLAG_PKS;
        }

        return 0;
}

int context_check_features(ARC_ProcessorFeatures *needed, ARC_ProcessorFeatures *avl) {
        uint64_t paging = needed->paging ^ avl->paging;
        paging &= needed->paging;

        if (paging == 0) {
                goto paging_ok;
        }

        // Check for mismatches with critical features

        if (ARC_CHECK_FEATURE(paging, ARC_PAGER_FLAG_PCID)) {
                // PCIDs aren't that critical, they can just be masked
                // out of paging tables when loading if they are not
                // supported on a particular processor
                goto paging_ok;
        }

        return -1;

        paging_ok:;

        uint64_t proc0 = needed->proc0 ^ avl->proc0;
        proc0 &= needed->proc0;

        if (proc0 == 0) {
                goto proc0_ok;
        }

        // TODO: Check critical stuff

        proc0_ok:;

        return 0;
}

ARC_Context *init_context(uint64_t flags) {
        ARC_Context *ret = alloc(sizeof(*ret));

        if (ret == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate context (Flags: 0x%"PRIx64")\n", flags);
                return NULL;
        }

        memset(ret, 0, sizeof(*ret));

        int xsave_space_size = 0;

        if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_XSAVE)) {
                xsave_space_size = 576; // Additional 64 bytes for xsave header
                // TODO: Add in other things that are enabled into size.
                //       This will most likely depend on the flags parameter
                //       and context_set_proc_features.
        } else if (ARC_CHECK_FEATURE(proc0, ARC_PROC0_FLAG_FXSAVE)){
                xsave_space_size = 512;
        }

        if (xsave_space_size >= 512) {
                void *xsave_space = alloc(xsave_space_size);

                if (xsave_space == NULL) {
                        ARC_DEBUG(ERR, "Failed to allocate xsave space\n");
                        free(ret);
                        return NULL;
                }

                memset(xsave_space, 0, xsave_space_size);

                ret->xsave_space = xsave_space;
        }

        ret->frame.gpr.cr0 = _x86_getCR0();
        ret->frame.gpr.cr4 = _x86_getCR4();

        if (MASKED_READ(flags, ARC_CONTEXT_FLAG_FLOATS, 1)
            && init_sse(ret) != 0) {
                free(ret);
                return NULL;
        }

        return ret;
}
