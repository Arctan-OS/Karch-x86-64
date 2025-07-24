%if 0
/**
 * @file smp.asm
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
 * The assembly trampoline to get APs synchronized with the BSP for x86-64.
*/
%endif

AP_PML4_OFF equ (_AP_START_INFO.pml4 - _AP_START_BEGIN)
AP_ENTRY_OFF equ (_AP_START_INFO.entry - _AP_START_BEGIN)
AP_FLAGS_OFF equ (_AP_START_INFO.flags - _AP_START_BEGIN)
AP_GDTR_OFF equ (_AP_START_INFO.gdtr - _AP_START_BEGIN)
AP_STACK_OFF equ (_AP_START_INFO.stack - _AP_START_BEGIN)
AP_EDX_OFF equ (_AP_START_INFO.edx - _AP_START_BEGIN)
AP_EAX_OFF equ (_AP_START_INFO.eax - _AP_START_BEGIN)
AP_PAT_OFF equ (_AP_START_INFO.pat - _AP_START_BEGIN)
AP_STACK_HIGH_OFF equ (_AP_START_INFO.stack_high - _AP_START_BEGIN)

section .rodata

bits 16
global _AP_START_BEGIN
_AP_START_BEGIN:
        cli
        cld

        mov bx, cs
        mov ds, bx
        mov fs, bx
        mov gs, bx
        mov es, bx
        mov ss, bx

        ;; Respond to BSP
        mov dword [ds:AP_EAX_OFF], eax
        mov dword [ds:AP_EDX_OFF], edx

        mov eax, dword [ds:AP_FLAGS_OFF]
        or eax, 0b10
        mov dword [ds:AP_FLAGS_OFF], eax

        ;; Setup stack
        mov ebp, dword [ds:AP_STACK_OFF]
        mov esp, ebp

        ;; Setup GDT
        o32 lgdt [ds:AP_GDTR_OFF]
        ;; Far jump to PM
        shl ebx, 4
        mov ecx, ebx
        add ebx, (pm - _AP_START_BEGIN)

        ;; Enable protected mode
        mov eax, 0x11
        mov cr0, eax

        push 0x8
        push bx
        retf

bits 32
pm:
        mov ax, 0x10
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov es, ax
        mov ss, ax

        ;; Setup long mode
        mov eax, cr4
        or eax, 1 << 5
        mov cr4, eax

        ;; PML4
        mov eax, dword [ecx + AP_PML4_OFF]
        mov cr3, eax

        ;; LME
        push ecx
        mov ecx, 0xC0000080
        rdmsr
        or eax, 1 << 8
        wrmsr
        pop ecx

        ;; Set paging
        mov eax, cr0
        or eax, 1 << 31
        mov cr0, eax

        lea eax, [ecx + (lm - _AP_START_BEGIN)]
        push 0x18
        push eax
        retf

extern Arc_ProcessorList
bits 64
lm:
        mov ax, 0x10
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov es, ax
        mov ss, ax

        mov eax, dword [rcx + AP_FLAGS_OFF]
        push rax
        and eax, 0b1000
        jz .no_nx

        push rcx
        mov ecx, 0xC0000080
        rdmsr
        or eax, 1 << 11
        wrmsr
        pop rcx

.no_nx:
        pop rax
        and eax, 0b0100
        jz .no_pat

        push rcx

        mov eax, dword [rcx + AP_PAT_OFF]
        mov edx, dword [rcx + AP_PAT_OFF + 4]
        mov ecx, 0x277
        wrmsr
       
        pop rcx

.no_pat:

        mov rbp, qword [rcx + AP_STACK_HIGH_OFF]
        mov rsp, rbp

        mov rbx, qword [rcx + AP_ENTRY_OFF]
        mov rdi, rcx
        add rdi, AP_PML4_OFF

        jmp rbx
        jmp $

global _AP_START_INFO
align 8
_AP_START_INFO:
        .pml4:
                dq 0x0
        .entry:
                dq 0x0
        .flags:
                dd 0x0
        .gdtr:
                dw 0x0
                dq 0x0
        .gdt_table:
                dq 0x0000000000000000
                dq 0x00CF9A000000FFFF
                dq 0x00CF92000000FFFF
                dq 0x00AF9A000000FFFF
        .stack:
                dq 0x0
        .edx:
                dd 0x0
        .eax:
                dd 0x0
        .pat:
                dq 0x0
        .stack_high:
                dq 0x0

global _AP_START_END
_AP_START_END:
