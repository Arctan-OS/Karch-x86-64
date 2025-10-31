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
#include "global.h"
#include "mm/allocator.h"

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
        // TODO: xsave

}

void context_load(ARC_Context *ctx, ARC_InterruptFrame *to) {
        memcpy(to, &ctx->frame, sizeof(*to));
        _x86_WRMSR(FS_BASE_MSR, (uintptr_t)ctx->tcb);
        // TODO: xrstor
}

int uninit_context(ARC_Context *context) {
        if (context == NULL) {
                return -1;
        }

        free(context->fxsave_space);
        free(context);

        return 0;
}

int context_set_proc_features() {
        return 0;
}

ARC_Context *init_context(uint64_t flags) {
        ARC_Context *ret = alloc(sizeof(*ret));

        if (ret == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate context (Flags: 0x%"PRIx64")\n", flags);
                return NULL;
        }

        memset(ret, 0, sizeof(*ret));

        if (MASKED_READ(flags, ARC_CONTEXT_FLAG_FLOATS, 1)) {
                ret->frame.gpr.cr0 = _x86_getCR0();
                ret->frame.gpr.cr4 = _x86_getCR4();

                if (init_sse(ret) != 0) {
                        free(ret);
                        return NULL;
                }
        }

        return ret;
}
