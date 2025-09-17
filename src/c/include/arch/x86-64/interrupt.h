/**
 * @file interrupt.h
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
#ifndef ARC_ARCH_X86_64_INTERRUPT_H
#define ARC_ARCH_X86_64_INTERRUPT_H

#include <stdint.h>

// TODO: Using printf in an interrupt (that doesn't panic the kernel) will cause
//       a deadlock if anything else is printing. So, code that is called from an
//       interrupt handler should not use printfs. The best way to resolve this is to
//       make it so that printfs do not deadlock.

// NOTE: This function will automatically push an additional value (0) to the stack to
//       take the place of an error code such that the interrupt frame structure does
//       not have to change
//
//       This should be used for interrupt vectors 8, 10, 12, 13, 14, 17, and 21; all
//       other vectors, including all IRQs from 32 to 255 should be fine to use this
//       macro
#define ARC_DEFINE_IRQ_HANDLER(_handler) \
        void __attribute__((naked)) ARC_NAME_IRQ(_handler)() { \
                __asm__("push 0"); \
                ARC_ASM_PUSH_ALL \
                __asm__("mov ax, 0x10; mov ss, ax"); \
                __asm__("mov ax, cs; cmp ax, [rsp + 160]; je 1f; swapgs; 1:"); \
                __asm__("mov rdi, rsp; call %0" :: "i"(_handler) :); \
                __asm__("mov ax, cs; cmp ax, [rsp + 160]; je 1f; swapgs; 1:"); \
                ARC_ASM_POP_ALL \
                __asm__("add rsp, 8; iretq"); \
        }

typedef struct ARC_IDTEntry {
        uint16_t offset1;
        uint16_t segment;
        uint8_t ist;
        uint8_t attrs;
        uint16_t offset2;
        uint32_t offset3;
        uint32_t reserved;
} __attribute__((packed)) ARC_IDTEntry;

typedef struct ARC_IDTRegister {
        uint16_t limit;
        uint64_t base;
} __attribute__((packed)) ARC_IDTRegister;

void install_idt_gate(ARC_IDTEntry *entry, uint64_t offset, uint16_t segment, uint8_t attrs, int ist);
int internal_init_early_exceptions(ARC_IDTEntry *entries, uint16_t kcode_seg, uint8_t ist);

#endif
