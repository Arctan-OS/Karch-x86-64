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
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/smp.h"

#define FS_BASE_MSR  0xC0000000
#define GS_BASE_MSR  0xC0000101
#define KGS_BASE_MSR 0xC0000102

void context_set_tcb(ARC_Context *ctx, void *tcb) {
        _x86_WRMSR(GS_BASE_MSR, (uintptr_t)tcb);
        ctx->tcb = tcb;
}

void *context_get_tcb(ARC_Context *ctx) {
        return ctx->tcb;
}

void context_set_proc_desc(ARC_x64ProcessorDescriptor *desc) {
        _x86_WRMSR(KGS_BASE_MSR, (uintptr_t)desc);
}
