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
#include "global.h"
#include "mm/allocator.h"

#define FS_BASE_MSR  0xC0000100
#define GS_BASE_MSR  0xC0000101
#define KGS_BASE_MSR 0xC0000102

void context_set_tcb(ARC_Context *ctx, void *tcb) {
        __asm__("swapgs");
        _x86_WRMSR(FS_BASE_MSR, (uintptr_t)tcb);
        __asm__("swapgs");
        ctx->tcb = tcb;
}

void *context_get_tcb(ARC_Context *ctx) {
        return ctx->tcb;
}

void context_set_proc_desc(ARC_x64ProcessorDescriptor *desc) {
        _x86_WRMSR(KGS_BASE_MSR, (uintptr_t)desc);
}

int uninit_context(ARC_Context *context) {
        if (context == NULL) {
                return -1;
        }

        free(context->fxsave_space);
        free(context);

        return 0;
}

ARC_Context *init_context(uint64_t flags) {
        ARC_Context *ret = alloc(sizeof(*ret));

        if (ret == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate context (Flags: 0x%"PRIx64")\n", flags);
                return NULL;
        }

        memset(ret, 0, sizeof(*ret));

        void *fxsave_space = alloc(512);

        if (fxsave_space == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate fxsave space\n");
        }

        memset(fxsave_space, 0, 512);

        ret->fxsave_space = fxsave_space;

        return ret;
}
