%if 0
/**
 * @file idt.asm
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Kernel - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Kernel.
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

global _install_idt
extern idtr
_install_idt:
        lidt [rdi]
        xor rax, rax
        ret

%include "src/asm/context.asm"

extern Arc_KernelPageTables
%macro common_idt_stub 1
section .text
global _idt_stub_%1
extern generic_interrupt_handler_%1
_idt_stub_%1:
%if %1 != 8 && %1 != 10 && %1 != 12 && %1 != 13 && %1 != 14 && %1 != 17 && %1 != 21
        ;; In the event that the interrupt does not produce an error code,
        ;; push a dummy error code to make stack interpretation a little easier
        ;; in C land
        push 0
%endif
        PUSH_ALL

        mov rdi, rsp

        mov ax, 0x10
        mov ss, ax

        call generic_interrupt_handler_%1

        POP_ALL
        add rsp, 8
        iretq
%endmacro

common_idt_stub 0
common_idt_stub 1
common_idt_stub 2
common_idt_stub 3
common_idt_stub 4
common_idt_stub 5
common_idt_stub 6
common_idt_stub 7
common_idt_stub 8
common_idt_stub 9
common_idt_stub 10
common_idt_stub 11
common_idt_stub 12
common_idt_stub 13
common_idt_stub 14
common_idt_stub 15
common_idt_stub 16
common_idt_stub 17
common_idt_stub 18
common_idt_stub 19
common_idt_stub 20
common_idt_stub 21
common_idt_stub 22
common_idt_stub 23
common_idt_stub 24
common_idt_stub 25
common_idt_stub 26
common_idt_stub 27
common_idt_stub 28
common_idt_stub 29
common_idt_stub 30
common_idt_stub 31
