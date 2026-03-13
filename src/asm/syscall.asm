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

%define PTR 0
%define BASE 8
%define LOCK 16
  
global _syscall
extern Arc_SyscallTable
extern Arc_KernelPageTables
extern syscall_get_kpages
extern syscall_get_stack
extern syscall_free_stack 
_syscall:
        cli
        swapgs

        lock inc [gs:LOCK]
        push rdx                ; +16
        mov rdx, [gs:PTR]
        sub rdx, 8
        mov [gs:PTR], rdx
        mov rdx, [rdx]
        push rdx                ; +8
        xchg rsp, rdx
        lock dec [gs:LOCK]
        
        ;; Save user context
        push 0                  ; SS
        push rdx                ; User stack
        push r11                ; RFLAGS
        push 0                  ; CS
        push rcx                ; Return address
        push 0                  ; Dummy error code
        mov rdx, [rdx + 16]     ; Restore value of RDX
        PUSH_ALL                ; Save user context
        
        PUSHAQ
        call syscall_get_kpages
        mov cr3, rax
        ;; Allocate new syscall stack for next syscall and push it to stack
        lock inc [gs:LOCK]
        mov rax, [gs:PTR]
        cmp rax, [gs:BASE]
        jg .over
        lock dec [gs:LOCK]        
        call syscall_get_stack
        lock inc [gs:LOCK]
        mov rbx, [gs:PTR]
        add rbx, 8
        mov [rbx], rax
        mov [gs:PTR], rbx
.over:
        lock dec [gs:LOCK]
        POPAQ
        
        ;; Figure out what handler to call
        shl rax, 3
        mov r12, Arc_SyscallTable
        add rax, r12

        sti
        
        ;; Invoke handler, set caller's return code
        call [rax]
        mov qword [rsp + 24], rax

        ;; Restore user context
        POP_ALL
        add rsp, 8
        pop rcx
        add rsp, 8
        pop r11
        mov rdi, rsp
        add rdx, 8
        pop rsp

        pop rdx                 ; +8
        push rax
        lock inc [gs:LOCK]
        mov rax, [gs:PTR]
        add rax, 8
        mov [rax], rdx
        mov [gs:PTR], rax
        lock dec [gs:LOCK]
        pop rax
        pop rdx                 ; +16
        
        swapgs
        
        o64 sysret
