/**
 * @file interrupt.h
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
#ifndef ARC_ARCH_X86_64_INTERRUPT_H
#define ARC_ARCH_X86_64_INTERRUPT_H

#include "arch/interrupt.h"
#include "arch/pager.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/context.h"
#include "arch/x86-64/util.h"
#include "interface/printf.h"
#include "lib/atomics.h"

#include <inttypes.h>
#include <stdint.h>

// TODO: Using printf in an interrupt (that doesn't panic the kernel) will cause
//       a deadlock if anything else is printing. So, code that is called from an
//       interrupt handler should not use printfs. The best way to resolve this is to
//       make it so that printfs do not deadlock.

#define GENERIC_HANDLER(_vector)                                        \
        extern void _idt_stub_##_vector();                              \
        void generic_interrupt_handler_##_vector(ARC_Registers *regs)

#define GENERIC_HANDLER_PREAMBLE(_vector)                               \
        pager_switch_to_kpages();                                       \
        int processor_id = lapic_get_id();                              \
        ARC_IDTFrame *interrupt_frame = (ARC_IDTFrame *)regs->rsp; \
        (void)processor_id;                                             \
        (void)interrupt_frame;

#define GENERIC_EXCEPTION_PREAMBLE(_vector)                             \
        uint64_t interrupt_error_code = 0;                              \
        (void)interrupt_error_code;                                     \
        switch (_vector) {                                              \
                case 8:                                                         \
                case 10:                                                \
                case 11:                                                \
                case 12:                                                \
                case 13:                                                \
                case 14:                                                \
                case 17:                                                \
                case 21:                                                \
                        interrupt_error_code = *(uint64_t *)regs->rsp;  \
                        regs->rsp += 0x8;                               \
                        break;                                          \
                default:                                                \
                        break;                                          \
        }                                                               \
        GENERIC_HANDLER_PREAMBLE(_vector)                               \

#define GENERIC_EXCEPTION_REG_DUMP(_vector) \
        spinlock_lock(&panic_lock);                                     \
        printf("Received Interrupt %d (%s) from LAPIC %d\n", _vector,   \
               exception_names[_vector], processor_id);                 \
        printf("RAX: 0x%016" PRIx64 "\n", regs->rax);                   \
        printf("RBX: 0x%016" PRIx64 "\n", regs->rbx);                   \
        printf("RCX: 0x%016" PRIx64 "\n", regs->rcx);                   \
        printf("RDX: 0x%016" PRIx64 "\n", regs->rdx);                   \
        printf("RSI: 0x%016" PRIx64 "\n", regs->rsi);                   \
        printf("RDI: 0x%016" PRIx64 "\n", regs->rdi);                   \
        printf("RSP: 0x%016" PRIx64 "\tSS: 0x%" PRIx64 "\n", regs->rsp,         \
               interrupt_frame->ss);                                    \
        printf("RBP: 0x%016" PRIx64 "\n", regs->rbp);                   \
        printf("R8 : 0x%016" PRIx64 "\n", regs->r8);                    \
        printf("R9 : 0x%016" PRIx64 "\n", regs->r9);                    \
        printf("R10: 0x%016" PRIx64 "\n", regs->r10);                   \
        printf("R11: 0x%016" PRIx64 "\n", regs->r11);                   \
        printf("R12: 0x%016" PRIx64 "\n", regs->r12);                   \
        printf("R13: 0x%016" PRIx64 "\n", regs->r13);                   \
        printf("R14: 0x%016" PRIx64 "\n", regs->r14);                   \
        printf("R15: 0x%016" PRIx64 "\n", regs->r15);                   \
        printf("RFLAGS: 0x016%" PRIx64 "\n", interrupt_frame->rflags);  \
        printf("Return address: 0x%"PRIx64":0x%016"PRIx64"\n", interrupt_frame->cs, \
               interrupt_frame->rip);                                   \
        printf("Error code: 0x%"PRIx64"\n", interrupt_error_code);      \

#define GENERIC_HANDLER_POSTAMBLE(_vector)      \
        lapic_eoi();

#define GENERIC_HANDLER_INSTALL(_entries, _vector, _segment, _ist)        \
        install_idt_gate(&_entries[_vector], (uintptr_t)&_idt_stub_##_vector, _segment, 0x8E, _ist);

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

static const char *exception_names[] = {
        "Division Error (#DE)",
        "Debug Exception (#DB)",
        "NMI",
        "Breakpoint (#BP)",
        "Overflow (#OF)",
        "BOUND Range Exceeded (#BR)",
        "Invalid Opcode (#UD)",
        "Device Not Available (No Math Coprocessor) (#NM)",
        "Double Fault (#DF)",
        "Coprocessor Segment Overrun (Reserved)",
        "Invalid TSS (#TS)",
        "Segment Not Present (#NP)",
        "Stack-Segment Fault (#SS)",
        "General Protection (#GP)",
        "Page Fault (#PF)",
        "Reserved",
        "x87 FPU Floating-Point Error (Math Fault) (#MF)",
        "Alignment Check (#AC)",
        "Machine Check (#MC)",
        "SIMD Floating-Point Exception (#XM)",
        "Virtualization Exception (#VE)",
        "Control Protection Exception (#CP)",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
};

extern ARC_IDTEntry idt_entries[32];
extern ARC_IDTRegister idt_register;

void install_idt_gate(ARC_IDTEntry *entry, uint64_t offset, uint16_t segment, uint8_t attrs, int ist);
int internal_init_early_exceptions(ARC_IDTEntry *entries, uint16_t kcode_seg, uint8_t ist);

#endif
