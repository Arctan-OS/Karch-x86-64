/**
 * @file idt.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan.
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
 * The file which handles the 64-bit IDT.
*/
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <arch/x86-64/idt.h>
#include <interface/printf.h>
#include <lib/util.h>
#include <arch/x86-64/apic/lapic.h>
#include <arch/x86-64/context.h>
#include <arch/smp.h>

#define GENERIC_HANDLER(__vector)					\
	extern void _idt_stub_##__vector();				\
	int generic_interrupt_handler_##__vector(struct ARC_Registers *regs)

#define GENERIC_HANDLER_PREAMBLE(__vector)				\
	int processor_id = lapic_get_id();				\
	struct interrupt_frame *interrupt_frame = (struct interrupt_frame *)regs->rsp; \
	(void)processor_id;						\
	(void)interrupt_frame;

#define GENERIC_EXCEPTION_PREAMBLE(__vector)				\
	uint64_t interrupt_error_code = 0;				\
	(void)interrupt_error_code;					\
	switch (__vector) {						\
		case 8:							\
		case 10:						\
		case 11:						\
		case 12:						\
		case 13:						\
		case 14:						\
		case 17:						\
		case 21:						\
			interrupt_error_code = *(uint64_t *)regs->rsp;	\
			regs->rsp += 0x8;				\
			break;						\
		default:						\
			break;						\
	}								\
	GENERIC_HANDLER_PREAMBLE(__vector)				\

#define GENERIC_EXCEPTION_REG_DUMP(__vector) \
	spinlock_lock(&panic_lock);					\
	printf("Received Interrupt %d (%s) from LAPIC %d\n", __vector,	\
	       exception_names[__vector], processor_id);		\
	printf("RAX: 0x%016" PRIx64 "\n", regs->rax);			\
	printf("RBX: 0x%016" PRIx64 "\n", regs->rbx);			\
	printf("RCX: 0x%016" PRIx64 "\n", regs->rcx);			\
	printf("RDX: 0x%016" PRIx64 "\n", regs->rdx);			\
	printf("RSI: 0x%016" PRIx64 "\n", regs->rsi);			\
	printf("RDI: 0x%016" PRIx64 "\n", regs->rdi);			\
	printf("RSP: 0x%016" PRIx64 "\tSS: 0x%" PRIx64 "\n", regs->rsp,	\
	       interrupt_frame->ss);					\
	printf("RBP: 0x%016" PRIx64 "\n", regs->rbp);			\
	printf("R8 : 0x%016" PRIx64 "\n", regs->r8);			\
	printf("R9 : 0x%016" PRIx64 "\n", regs->r9);			\
	printf("R10: 0x%016" PRIx64 "\n", regs->r10);			\
	printf("R11: 0x%016" PRIx64 "\n", regs->r11);			\
	printf("R12: 0x%016" PRIx64 "\n", regs->r12);			\
	printf("R13: 0x%016" PRIx64 "\n", regs->r13);			\
	printf("R14: 0x%016" PRIx64 "\n", regs->r14);			\
	printf("R15: 0x%016" PRIx64 "\n", regs->r15);			\
	printf("RFLAGS: 0x016%" PRIx64 "\n", interrupt_frame->rflags);	\
	printf("Return address: 0x%"PRIx64":0x%016"PRIx64"\n", interrupt_frame->cs, \
	       interrupt_frame->rip);					\
	printf("Error code: 0x%"PRIx64"\n", interrupt_error_code);	\
	memset(Arc_MainTerm.framebuffer, 0,				\
	       Arc_MainTerm.fb_width *Arc_MainTerm.fb_height *(Arc_MainTerm.fb_bpp / \
							       8));	\
	term_draw(&Arc_MainTerm);

#define GENERIC_HANDLER_POSTAMBLE(__vector)	\
	lapic_eoi();

#define GENERIC_HANDLER_INSTALL(__vector)	\
	install_idt_gate(__vector, (uintptr_t)&_idt_stub_##__vector, 0x08, 0x8E);

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

static ARC_GenericSpinlock panic_lock;

struct idt_desc {
	uint16_t limit;
	uint64_t base;
}__attribute__((packed));
struct idt_desc idtr;

struct idt_entry {
	uint16_t offset1;
	uint16_t segment;
	uint8_t ist;
	uint8_t attrs;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t reserved;
}__attribute__((packed));
static struct idt_entry idt_entries[256];

struct interrupt_frame {
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
}__attribute__((packed));
STATIC_ASSERT(sizeof(struct interrupt_frame) == 40 , "Interrupt frame wrong size");

GENERIC_HANDLER(0) {
	GENERIC_EXCEPTION_PREAMBLE(0);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(0);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(1) {
	GENERIC_EXCEPTION_PREAMBLE(1);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(1);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(2) {
	GENERIC_EXCEPTION_PREAMBLE(2);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(2);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(3) {
	GENERIC_EXCEPTION_PREAMBLE(3);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(3);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(4) {
	GENERIC_EXCEPTION_PREAMBLE(4);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(4);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(5) {
	GENERIC_EXCEPTION_PREAMBLE(5);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(5);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(6) {
	GENERIC_EXCEPTION_PREAMBLE(6);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(6);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(7) {
	GENERIC_EXCEPTION_PREAMBLE(7);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(7);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(8) {
	GENERIC_EXCEPTION_PREAMBLE(8);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(8);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(9) {
	GENERIC_EXCEPTION_PREAMBLE(9);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(9);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(10) {
	GENERIC_EXCEPTION_PREAMBLE(10);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(10);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(11) {
	GENERIC_EXCEPTION_PREAMBLE(11);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(11);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(12) {
	GENERIC_EXCEPTION_PREAMBLE(12);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(12);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(13) {
	GENERIC_EXCEPTION_PREAMBLE(13);
	GENERIC_EXCEPTION_REG_DUMP(13);
	if (interrupt_error_code == 0) {
		printf("#GP may have been caused by one of the following:\n");
		printf("\tAn operand of the instruction\n");
		printf("\tA selector from a gate which is the operand of the instruction\n");
		printf("\tA selector from a TSS involved in a task switch\n");
		printf("\tIDT vector number\n");
	}

	printf("Error code 0x%"PRIx64"\n", interrupt_error_code);

	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(14) {
	GENERIC_EXCEPTION_PREAMBLE(14);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(14);
	_x86_getCR2();
	printf("CR2: 0x%016"PRIx64"\n", _x86_CR2);
	_x86_getCR3();
	printf("CR3: 0x%016"PRIx64"\n", _x86_CR3);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(15) {
	GENERIC_EXCEPTION_PREAMBLE(15);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(15);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(16) {
	GENERIC_EXCEPTION_PREAMBLE(16);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(16);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(17) {
	GENERIC_EXCEPTION_PREAMBLE(17);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(17);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(18) {
	GENERIC_EXCEPTION_PREAMBLE(18);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(18);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(19) {
	GENERIC_EXCEPTION_PREAMBLE(19);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(19);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(20) {
	GENERIC_EXCEPTION_PREAMBLE(20);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(20);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(21) {
	GENERIC_EXCEPTION_PREAMBLE(21);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(21);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(22) {
	GENERIC_EXCEPTION_PREAMBLE(22);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(22);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(23) {
	GENERIC_EXCEPTION_PREAMBLE(23);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(23);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(24) {
	GENERIC_EXCEPTION_PREAMBLE(24);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(24);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(25) {
	GENERIC_EXCEPTION_PREAMBLE(25);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(25);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(26) {
	GENERIC_EXCEPTION_PREAMBLE(26);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(26);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(27) {
	GENERIC_EXCEPTION_PREAMBLE(27);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(27);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(28) {
	GENERIC_EXCEPTION_PREAMBLE(28);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(28);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(29) {
	GENERIC_EXCEPTION_PREAMBLE(29);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(29);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(30) {
	GENERIC_EXCEPTION_PREAMBLE(30);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(30);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(31) {
	GENERIC_EXCEPTION_PREAMBLE(31);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(31);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
	return 0;
}

GENERIC_HANDLER(32) {
	GENERIC_HANDLER_PREAMBLE(32);

	if (processor_id == -1) {
		GENERIC_HANDLER_POSTAMBLE(32);
		return 0;
	}

	struct ARC_ProcessorDescriptor *processor = &Arc_ProcessorList[processor_id];

	if ((processor->generic.flags & 1) == 1) {
		// Context switch
		struct ARC_Registers current = { 0 };

		// Save current state
		current.rax = regs->rax;
		current.rbx = regs->rbx;
		current.rcx = regs->rcx;
		current.rdx = regs->rdx;
		current.rsi = regs->rsi;
		current.rdi = regs->rdi;
		current.rsp = interrupt_frame->rsp;
		current.rbp = regs->rbp;
		current.rip = regs->rip;
		current.r8 = regs->r8;
		current.r9 = regs->r9;
		current.r10 = regs->r10;
		current.r11 = regs->r11;
		current.r12 = regs->r12;
		current.r13 = regs->r13;
		current.r14 = regs->r14;
		current.r15 = regs->r15;
		current.cs = interrupt_frame->cs;
		current.rip = interrupt_frame->rip;
		current.rflags = interrupt_frame->rflags;
		current.ss = interrupt_frame->ss;

		// Accept changes
		regs->rax = processor->generic.registers.rax;
		regs->rbx = processor->generic.registers.rbx;
		regs->rcx = processor->generic.registers.rcx;
		regs->rdx = processor->generic.registers.rdx;
		regs->rsi = processor->generic.registers.rsi;
		regs->rdi = processor->generic.registers.rdi;
		interrupt_frame->rsp = processor->generic.registers.rsp;
		regs->rbp = processor->generic.registers.rbp;
		regs->rip = processor->generic.registers.rip;
		regs->r8 = processor->generic.registers.r8;
		regs->r9 = processor->generic.registers.r9;
		regs->r10 = processor->generic.registers.r10;
		regs->r11 = processor->generic.registers.r11;
		regs->r12 = processor->generic.registers.r12;
		regs->r13 = processor->generic.registers.r13;
		regs->r14 = processor->generic.registers.r14;
		regs->r15 = processor->generic.registers.r15;
		interrupt_frame->cs = processor->generic.registers.cs;
		interrupt_frame->rip = processor->generic.registers.rip;
		interrupt_frame->rflags = processor->generic.registers.rflags;
		interrupt_frame->ss = processor->generic.registers.ss;

		// Write back current state
		smp_context_write(processor, &current);
		processor->generic.flags &= ~1;
	}

	if (((processor->generic.flags >> 1) & 1) == 1) {
		// Write back current state
		processor->generic.registers.rax = regs->rax;
		processor->generic.registers.rbx = regs->rbx;
		processor->generic.registers.rcx = regs->rcx;
		processor->generic.registers.rdx = regs->rdx;
		processor->generic.registers.rsi = regs->rsi;
		processor->generic.registers.rdi = regs->rdi;
		processor->generic.registers.rsp = interrupt_frame->rsp;
		processor->generic.registers.rbp = regs->rbp;
		processor->generic.registers.rip = regs->rip;
		processor->generic.registers.r8 = regs->r8;
		processor->generic.registers.r9 = regs->r9;
		processor->generic.registers.r10 = regs->r10;
		processor->generic.registers.r11 = regs->r11;
		processor->generic.registers.r12 = regs->r12;
		processor->generic.registers.r13 = regs->r13;
		processor->generic.registers.r14 = regs->r14;
		processor->generic.registers.r15 = regs->r15;
		processor->generic.registers.cs = interrupt_frame->cs;
		processor->generic.registers.rip = interrupt_frame->rip;
		processor->generic.registers.rflags = interrupt_frame->rflags;
		processor->generic.registers.ss = interrupt_frame->ss;

		processor->generic.flags &= ~(1 << 1);
	}

	mutex_unlock(&processor->generic.register_lock);

	mutex_lock(&processor->generic.timer_lock);

	if (processor->generic.timer_mode == ARC_LAPIC_TIMER_ONESHOT) {
		lapic_refresh_timer(processor->generic.timer_ticks);
	}

	if (((processor->generic.flags >> 2) & 1) == 1) {
		lapic_setup_timer(32, processor->generic.timer_mode);
		lapic_refresh_timer(processor->generic.timer_ticks);

		processor->generic.flags &= ~(1 << 2);
	}

	mutex_unlock(&processor->generic.timer_lock);


	GENERIC_HANDLER_POSTAMBLE(32);
	return 0;
}

GENERIC_HANDLER(33) {
	GENERIC_HANDLER_PREAMBLE(33);
	GENERIC_HANDLER_POSTAMBLE(33);
	return 0;
}

GENERIC_HANDLER(34) {
	GENERIC_HANDLER_PREAMBLE(34);
	GENERIC_HANDLER_POSTAMBLE(34);
	return 0;
}

GENERIC_HANDLER(35) {
	GENERIC_HANDLER_PREAMBLE(35);
	GENERIC_HANDLER_POSTAMBLE(35);
	return 0;
}

GENERIC_HANDLER(36) {
	GENERIC_HANDLER_PREAMBLE(36);
	GENERIC_HANDLER_POSTAMBLE(36);
	return 0;
}

GENERIC_HANDLER(37) {
	GENERIC_HANDLER_PREAMBLE(37);
	GENERIC_HANDLER_POSTAMBLE(37);
	return 0;
}

GENERIC_HANDLER(38) {
	GENERIC_HANDLER_PREAMBLE(38);
	GENERIC_HANDLER_POSTAMBLE(38);
	return 0;
}

GENERIC_HANDLER(39) {
	GENERIC_HANDLER_PREAMBLE(39);
	GENERIC_HANDLER_POSTAMBLE(39);
	return 0;
}

GENERIC_HANDLER(40) {
	GENERIC_HANDLER_PREAMBLE(40);
	GENERIC_HANDLER_POSTAMBLE(40);
	return 0;
}

GENERIC_HANDLER(41) {
	GENERIC_HANDLER_PREAMBLE(41);
	GENERIC_HANDLER_POSTAMBLE(41);
	return 0;
}

GENERIC_HANDLER(42) {
	GENERIC_HANDLER_PREAMBLE(42);
	GENERIC_HANDLER_POSTAMBLE(42);
	return 0;
}

GENERIC_HANDLER(43) {
	GENERIC_HANDLER_PREAMBLE(43);
	GENERIC_HANDLER_POSTAMBLE(43);
	return 0;
}

GENERIC_HANDLER(44) {
	GENERIC_HANDLER_PREAMBLE(44);
	GENERIC_HANDLER_POSTAMBLE(44);
	return 0;
}

GENERIC_HANDLER(45) {
	GENERIC_HANDLER_PREAMBLE(45);
	GENERIC_HANDLER_POSTAMBLE(45);
	return 0;
}

GENERIC_HANDLER(46) {
	GENERIC_HANDLER_PREAMBLE(46);
	GENERIC_HANDLER_POSTAMBLE(46);
	return 0;
}

GENERIC_HANDLER(47) {
	GENERIC_HANDLER_PREAMBLE(47);
	GENERIC_HANDLER_POSTAMBLE(47);
	return 0;
}

GENERIC_HANDLER(48) {
	GENERIC_HANDLER_PREAMBLE(48);
	GENERIC_HANDLER_POSTAMBLE(48);
	return 0;
}

GENERIC_HANDLER(49) {
	GENERIC_HANDLER_PREAMBLE(49);
	GENERIC_HANDLER_POSTAMBLE(49);
	return 0;
}

GENERIC_HANDLER(50) {
	GENERIC_HANDLER_PREAMBLE(50);
	GENERIC_HANDLER_POSTAMBLE(50);
	return 0;
}

void install_idt_gate(int i, uint64_t offset, uint16_t segment, uint8_t attrs) {
	idt_entries[i].offset1 = offset & 0xFFFF;
	idt_entries[i].offset2 = (offset >> 16) & 0xFFFF;
	idt_entries[i].offset3 = (offset >> 32) & 0xFFFFFFFF;
	idt_entries[i].segment = segment;
	idt_entries[i].attrs = attrs;
	idt_entries[i].ist = 1;
	idt_entries[i].reserved = 0;
}

void init_idt() {
	// Exception
	GENERIC_HANDLER_INSTALL(0);
	GENERIC_HANDLER_INSTALL(1);
	GENERIC_HANDLER_INSTALL(2);
	GENERIC_HANDLER_INSTALL(3);
	GENERIC_HANDLER_INSTALL(4);
	GENERIC_HANDLER_INSTALL(5);
	GENERIC_HANDLER_INSTALL(6);
	GENERIC_HANDLER_INSTALL(7);
	GENERIC_HANDLER_INSTALL(8);
	GENERIC_HANDLER_INSTALL(9);
	GENERIC_HANDLER_INSTALL(10);
	GENERIC_HANDLER_INSTALL(11);
	GENERIC_HANDLER_INSTALL(12);
	GENERIC_HANDLER_INSTALL(13);
	GENERIC_HANDLER_INSTALL(14);
	GENERIC_HANDLER_INSTALL(15);
	GENERIC_HANDLER_INSTALL(16);
	GENERIC_HANDLER_INSTALL(17);
	GENERIC_HANDLER_INSTALL(18);
	GENERIC_HANDLER_INSTALL(19);
	GENERIC_HANDLER_INSTALL(20);
	GENERIC_HANDLER_INSTALL(21);
	GENERIC_HANDLER_INSTALL(22);
	GENERIC_HANDLER_INSTALL(23);
	GENERIC_HANDLER_INSTALL(24);
	GENERIC_HANDLER_INSTALL(25);
	GENERIC_HANDLER_INSTALL(26);
	GENERIC_HANDLER_INSTALL(27);
	GENERIC_HANDLER_INSTALL(28);
	GENERIC_HANDLER_INSTALL(29);
	GENERIC_HANDLER_INSTALL(30);
	GENERIC_HANDLER_INSTALL(31);

	// IRQ
	GENERIC_HANDLER_INSTALL(32);
	GENERIC_HANDLER_INSTALL(33);
	GENERIC_HANDLER_INSTALL(34);
	GENERIC_HANDLER_INSTALL(35);
	GENERIC_HANDLER_INSTALL(36);
	GENERIC_HANDLER_INSTALL(37);
	GENERIC_HANDLER_INSTALL(38);
	GENERIC_HANDLER_INSTALL(39);
	GENERIC_HANDLER_INSTALL(40);
	GENERIC_HANDLER_INSTALL(41);
	GENERIC_HANDLER_INSTALL(42);
	GENERIC_HANDLER_INSTALL(43);
	GENERIC_HANDLER_INSTALL(44);
	GENERIC_HANDLER_INSTALL(45);
	GENERIC_HANDLER_INSTALL(46);
	GENERIC_HANDLER_INSTALL(47);
	GENERIC_HANDLER_INSTALL(48);
	GENERIC_HANDLER_INSTALL(49);
	GENERIC_HANDLER_INSTALL(50);
	
	idtr.limit = sizeof(idt_entries) * 16 - 1;
	idtr.base = (uintptr_t)&idt_entries;

	_install_idt();

	init_static_spinlock(&panic_lock);

	ARC_DEBUG(INFO, "Ported IDT to 64-bits\n");
}
