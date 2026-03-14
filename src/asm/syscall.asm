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
extern syscall_get_kstack
_syscall:
        cli
        swapgs

        push rax                ; + 8
        mov rax, cr3
        push rax                ; + 0

        call syscall_get_kpages
        mov cr3, rax
        
        call syscall_get_kstack
        xchg rax, rsp
        push rax
        
        ;; Save user context
        mov rax, [rax + 8]      ; Restore value of RAX
        PUSHAQ                  ; Save user context       
        
        ;; Figure out what handler to call
        mov r12, Arc_SyscallTable
        lea rax, [r12 + rax * 8]
        
        sti
        
        ;; Invoke handler, set caller's return code
        call [rax]
        mov qword [rsp], rax

        ;; Restore user context bar RAX
        POPAQ
        pop rsp

        mov [rsp + 8], rax
        pop rax
        mov cr3, rax
        pop rax
        
        swapgs
        
        o64 sysret
