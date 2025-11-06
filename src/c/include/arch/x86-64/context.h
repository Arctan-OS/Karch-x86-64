/**
 * @file context.h
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
#ifndef ARC_ARCH_X86_64_CONTEXT_H
#define ARC_ARCH_X86_64_CONTEXT_H

#include <stdint.h>

#define ARC_ASM_PUSH_ALL \
        asm("push rbp; \
        push r15; \
        push r14; \
        push r13; \
        push r12; \
        push r11; \
        push r10; \
        push r9; \
        push r8; \
        push rdi; \
        push rsi; \
        push rdx; \
        push rcx; \
        push rbx; \
        push rax; \
        mov r15, cr4; \
        push r15; \
        mov r15, cr3; \
        push r15; \
        mov r15, cr0; \
        push r15;");

#define ARC_ASM_POP_ALL \
        asm("pop r15;\
        mov cr0, r15; \
        pop r15; \
        mov cr3, r15;\
        pop r15; \
        mov cr4, r15; \
        pop rax; \
        pop rbx; \
        pop rcx; \
        pop rdx; \
        pop rsi; \
        pop rdi; \
        pop r8; \
        pop r9; \
        pop r10; \
        pop r11; \
        pop r12; \
        pop r13; \
        pop r14; \
        pop r15; \
        pop rbp;");

typedef struct ARC_Registers {
        uint64_t cr0;
	uint64_t cr3;
        uint64_t cr4;
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rbp;
}__attribute__((packed)) ARC_Registers;

typedef struct ARC_InterruptFrame {
	ARC_Registers gpr;
	uint64_t error;
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
} __attribute__((packed)) ARC_InterruptFrame;

typedef struct ARC_Context {
	void *xsave_space;
	void *tcb;
	ARC_InterruptFrame frame;
} ARC_Context;

#endif
