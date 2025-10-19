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
section .userspace

%include "src/asm/context.asm"

global _syscall
extern Arc_SyscallTable
extern Arc_KernelPageTables
extern syscall_get_kpages
_syscall:
        swapgs

        mov rdx, rsp            ; Save user RSP
        mov rsp, [gs:0]         ; Switch to kernel stack
        add rsp, 0x2000 - 16

        push 0                  ; SS
        push rdx                ; User stack
        push r11                ; RFLAGS
        push 0                  ; CS
        push rcx                ; Return address
        push 0                  ; Dummy error code
        PUSH_ALL                ; Save user context

        ;; TODO: Would be nice to get rid of this call
        ;;       such that it could just be:
        ;;       mov rax, [gs:<offset of descriptor pointer>]
        ;;       mov rax, [rax:<offset of process pointer>]
        ;;       mov rax, [rax:<offset of page_tables.kernel>]
        ;;       Problem: don't know those offsets, and don't
        ;;       know how to get them
        push rax
        call syscall_get_kpages
        mov cr3, rax
        pop rax

        ;; Figure out what handler to call
        shl rax, 3
        mov r12, Arc_SyscallTable
        add rax, r12

        ;; Invoke handler, set caller's return code
        call [rax]
        mov qword [rsp + 24], rax

        POP_ALL                 ;Restore user context
        add rsp, 8
        pop rcx
        add rsp, 8
        pop r11
        pop rsp

        swapgs

        o64 sysret
