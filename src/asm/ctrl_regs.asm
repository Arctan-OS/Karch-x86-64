%if 0
/**
 * @file ctrl_regs.asm
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
%macro GET_DATA 2
global _x86_get%1
_x86_get%1:
    mov rax, %1
    ret
%endmacro

GET_DATA CR0
GET_DATA CR1
GET_DATA CR2
GET_DATA CR3
GET_DATA CR4

global _x86_RDMSR
_x86_RDMSR:
        xor rax, rax
        xor rdx, rdx
        mov ecx, edi
        rdmsr
        rol rdx, 32
        or rax, rdx
        ret

section .text
%macro SET_DATA 2
global _x86_set%1
_x86_set%1:
    mov rax, rdi
    mov %1, rax
    ret
%endmacro

SET_DATA CR0
SET_DATA CR1
SET_DATA CR2
SET_DATA CR3
SET_DATA CR4

global _x86_WRMSR
_x86_WRMSR:
        mov ecx, edi
        mov eax, esi
        ror rsi, 32
        mov edx, esi
        wrmsr
        ret
