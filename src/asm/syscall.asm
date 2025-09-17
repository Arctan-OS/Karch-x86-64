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

        mov rdx, rsp            ; Save user RSP
        mov rsp, [gs:0]         ; Switch to kernel stack

        push 0                  ; SS
        push rdx                ; User stack
        push r11                ; RFLAGS
        push 0                  ; CS
        push rcx                ; Return address
        push 0                  ; Dummy error code
        PUSH_ALL                ; Save user context

        ;; Call handler
        shl rax, 3
        mov r12, Arc_SyscallTable
        add rax, r12
        call [rax]

        ;; Preserve RAX from here on, as it contains return code

        ;; Swap caller RAX for return code
        mov qword [rsp + 24], rax

        POP_ALL                 ;Restore user context
        add rsp, 8
        pop rcx
        add rsp, 8
        pop r11
        pop rsp

        swapgs

        o64 sysret
