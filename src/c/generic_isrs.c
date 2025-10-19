/**
 * @file generic_isrs.c
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
#include "arch/smp.h"
#include "arch/x86-64/interrupt.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/pager.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/context.h"
#include "arch/x86-64/util.h"
#include "config.h"
#include "global.h"
#include "interface/printf.h"
#include "lib/spinlock.h"

#define GENERIC_HANDLER(_vector)                                        \
        extern void _idt_stub_##_vector();                              \
        void USERSPACE generic_interrupt_handler_##_vector(ARC_InterruptFrame *frame)

#define GENERIC_HANDLER_PREAMBLE                               \
        int processor_id = lapic_get_id();                              \
        (void)processor_id;                                             \

#define GENERIC_EXCEPTION_REG_DUMP(_vector) \
        printf("Received Interrupt %d (%s) from LAPIC %d\n", _vector,   \
               exception_names[_vector], processor_id);                 \
        printf("RAX: 0x%016" PRIx64 "\n", frame->gpr.rax);                   \
        printf("RBX: 0x%016" PRIx64 "\n", frame->gpr.rbx);                   \
        printf("RCX: 0x%016" PRIx64 "\n", frame->gpr.rcx);                   \
        printf("RDX: 0x%016" PRIx64 "\n", frame->gpr.rdx);                   \
        printf("RSI: 0x%016" PRIx64 "\n", frame->gpr.rsi);                   \
        printf("RDI: 0x%016" PRIx64 "\n", frame->gpr.rdi);                   \
        printf("RSP: 0x%016" PRIx64 "\tSS: 0x%" PRIx64 "\n", frame->rsp,         \
               frame->ss);                                    \
        printf("RBP: 0x%016" PRIx64 "\n", frame->gpr.rbp);                   \
        printf("R8 : 0x%016" PRIx64 "\n", frame->gpr.r8);                    \
        printf("R9 : 0x%016" PRIx64 "\n", frame->gpr.r9);                    \
        printf("R10: 0x%016" PRIx64 "\n", frame->gpr.r10);                   \
        printf("R11: 0x%016" PRIx64 "\n", frame->gpr.r11);                   \
        printf("R12: 0x%016" PRIx64 "\n", frame->gpr.r12);                   \
        printf("R13: 0x%016" PRIx64 "\n", frame->gpr.r13);                   \
        printf("R14: 0x%016" PRIx64 "\n", frame->gpr.r14);                   \
        printf("R15: 0x%016" PRIx64 "\n", frame->gpr.r15);                   \
        printf("RFLAGS: 0x016%" PRIx64 "\n", frame->rflags);  \
        printf("Return address: 0x%"PRIx64":0x%016"PRIx64"\n", frame->cs, \
               frame->rip);                                   \
        printf("Error code: 0x%"PRIx64"\n", frame->error);      \

#define GENERIC_HANDLER_POSTAMBLE      \
        lapic_eoi();

#define GENERIC_HANDLER_INSTALL(_entries, _vector, _segment, _ist)        \
        install_idt_gate(&_entries[_vector], (uintptr_t)&_idt_stub_##_vector, _segment, 0x8E, _ist);

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

static ARC_Spinlock panic_lock;

GENERIC_HANDLER(0) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(0);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(1) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(1);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(2) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(2);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(3) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(3);
	spinlock_unlock(&panic_lock);
        GENERIC_HANDLER_POSTAMBLE;
}

GENERIC_HANDLER(4) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(4);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(5) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(5);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(6) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(6);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(7) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(7);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(8) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(8);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(9) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(9);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(10) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(10);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(11) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(11);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(12) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(12);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(13) {
	GENERIC_HANDLER_PREAMBLE;
	GENERIC_EXCEPTION_REG_DUMP(13);
	if (frame->error == 0) {
		printf("#GP may have been caused by one of the following:\n");
		printf("\tAn operand of the instruction\n");
		printf("\tA selector from a gate which is the operand of the instruction\n");
		printf("\tA selector from a TSS involved in a task switch\n");
		printf("\tIDT vector number\n");
	}

	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(14) {
	GENERIC_HANDLER_PREAMBLE;
        uintptr_t vaddr = _x86_getCR2();

        GENERIC_EXCEPTION_REG_DUMP(14);
        printf("CR2: 0x%016"PRIx64"\n", vaddr);
        printf("CR3: 0x%016"PRIx64"\n", frame->gpr.cr3);
        spinlock_unlock(&panic_lock);
        GENERIC_HANDLER_POSTAMBLE;
        ARC_HANG;

/*
        if (frame->cs == 0x8 && frame->gpr.cr3 != Arc_KernelPageTables) {
                uintptr_t paddr = vaddr;

                if (vaddr >= ARC_HHDM_VADDR) {
                        paddr = ARC_HHDM_TO_PHYS(vaddr);
                }
                printf("%p %p %p\n", frame->rip, vaddr, paddr);
                //pager_map((void *)ARC_PHYS_TO_HHDM(frame->gpr.cr3), vaddr, paddr, PAGE_SIZE, 1 << ARC_PAGER_RW);
        } else {
                }
*/
        GENERIC_HANDLER_POSTAMBLE;
}

GENERIC_HANDLER(15) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(15);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(16) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(16);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(17) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(17);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(18) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(18);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(19) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(19);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(20) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(20);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(21) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(21);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(22) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(22);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(23) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(23);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(24) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(24);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(25) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(25);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(26) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(26);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(27) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(27);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(28) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(28);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(29) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(29);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(30) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(30);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(31) {
	GENERIC_HANDLER_PREAMBLE;
        GENERIC_EXCEPTION_REG_DUMP(31);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

int internal_init_early_exceptions(ARC_IDTEntry *entries, uint16_t kcode_seg, uint8_t ist) {
	if (entries == NULL) {
		return -1;
	}

	GENERIC_HANDLER_INSTALL(entries, 0, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 1, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 2, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 3, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 4, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 5, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 6, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 7, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 8, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 9, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 10, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 11, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 12, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 13, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 14, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 15, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 16, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 17, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 18, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 19, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 20, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 21, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 22, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 23, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 24, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 25, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 26, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 27, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 28, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 29, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 30, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 31, kcode_seg, ist);

        return 0;
}
