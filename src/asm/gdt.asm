%if 0
/**
 * @file gdt.asm
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

extern gdtr
global _install_gdt
_install_gdt:
        cli
        lgdt [rdi]
        ;; Do a ret long jump to set CS
        push 0x08
        lea rax, [rel _gdt_set_cs]
        push rax
        retfq
_gdt_set_cs:
        ;; Set segments to 32/64-bit data offset
        mov ax, 0x10
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov es, ax
        xor rax, rax
        ret

global _install_tss
_install_tss:
        ltr di
        xor rax, rax
        ret
