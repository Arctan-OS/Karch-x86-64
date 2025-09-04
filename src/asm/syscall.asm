%if 0
/**
 * @file idt.asm
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
%endif

bits 64
section .text

%include "src/asm/context.asm"

global _syscall
extern Arc_SyscallTable
extern Arc_KernelPageTables
_syscall:
        swapgs
        ;; Save return address
        push rcx
        mov rcx, rsp
        mov rsp, [gs:0]
        ;; Save user stack
        push rcx
        ;; Save user context
        PUSH_ALL

        ;; Call handler
        shl rax, 3
        mov r12, Arc_SyscallTable
        add rax, r12
        call [rax]

        ;; Preserve RAX from here on, as it contains return code

        ;; Swap caller RAX for return code
        pop r11                 ; CR3
        pop r10                 ; RAX
        push rax                ; New RAX
        push r11                ; CR3
        ;; Restore user context
        POP_ALL
        ;; Restore user stack
        pop rsp
        ;; Restore return address
        pop rcx
        swapgs

        o64 sysret
