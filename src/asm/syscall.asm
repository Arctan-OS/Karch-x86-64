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

        ;; Restore caller state
        PUSH_ALL

        ;; Swap page tables
        mov r10, cr3
        lea rax, [rel Arc_KernelPageTables]
        mov rax, [rax]
        mov cr3, rax

        ;; Change stack
        ;; Unfortunately can't use rdgsbase rax
        mov r11, rcx
        mov r10, rdx
        swapgs
        mov ecx, 0xC0000101
        rdmsr
        mov rcx, r11
        mov rdx, r10

        ;; Have to do this silliness as upper 32-bits of processor
        ;; descriptor are not preserved in GS base
        xor r11, r11
        not r11
        shl r11, 32
        or rax, r11
        ;; Finally change the stack
        mov r11, rsp
        mov rsp, qword [rax + 8]
        push r11
        push rbp
        mov rbp, rsp
        push r10

        ;; Call handler
        shl rdi, 3
        mov rax, Arc_SyscallTable
        add rax, rdi
        mov rax, [rax]
        call rax
        ;; Preserve RAX from here on, as it contains return code

        ;; Resotre old page tables
        pop r10
        pop rcx
        pop rdx
        mov cr3, r10

        ;; Restore old stack
        mov rbp, rcx
        mov rsp, rdx
        swapgs

        ;; Restore caller state
        POP_ALL

        o64 sysret
