%if 0
/**
 * @file idt.asm
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
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
%endif

bits 64
section .text

%include "src/asm/context.asm"

global _syscall
extern Arc_SyscallTable
extern Arc_KernelPageTables
_syscall:
        cli

        ;; Get stack
        ;; Unfortunately can't use rdgsbase rax

        push r12
        push r10

        push rcx
        push rdx
        push rax
        swapgs
        mov ecx, 0xC0000101
        rdmsr
        swapgs
        mov r12, rax
        pop rax
        pop rdx
        pop rcx

        ;; Have to do this silliness as upper 32-bits of processor
        ;; descriptor are not preserved in GS base
        xor r10, r10
        not r10
        shl r10, 32
        or r12, r10
        ;; Finally change the stack
        mov r10, rsp
        mov rsp, qword [r12]
        ;; Push caller state except RSP
        push r10
        PUSH_ALL

        ;; Call handler
        shl rax, 3
        mov r12, Arc_SyscallTable
        add r12, rax
        mov r12, [r12]
        call r12

        ;; Preserve RAX from here on, as it contains return code

        ;; Swap caller RAX for return code
        pop r11                 ; CR3
        pop r10                 ; RAX
        push rax                ; New RAX
        push r11                ; CR3
        ;; Restore caller state except RSP
        POP_ALL
        pop rsp

        pop r10
        pop r12

        o64 sysret
