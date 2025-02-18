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
#include <mp/scheduler.h>
#include <arch/pager.h>

// TODO: Using printf in an interrupt (that doesn't panic the kernel) will cause
//       a deadlock if anything else is printing. So, code that is called from an
//       interrupt handler should not use printfs. The best way to resolve this is to
//       make it so that printfs do not deadlock.

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

	struct ARC_ProcessorDescriptor *processor = smp_get_proc_desc();

	struct ARC_Registers *proc_regs = &processor->registers;

	pager_switch_to_kpages();

	if (MASKED_READ(processor->flags, ARC_SMP_FLAGS_CTXSAVE, 1) == 1) {
		proc_regs->rax = regs->rax;
		proc_regs->rbx = regs->rbx;
		proc_regs->rcx = regs->rcx;
		proc_regs->rdx = regs->rdx;
		proc_regs->rsi = regs->rsi;
		proc_regs->rdi = regs->rdi;
		proc_regs->rsp = interrupt_frame->rsp;
		proc_regs->rbp = regs->rbp;
		proc_regs->rip = regs->rip;
		proc_regs->r8 = regs->r8;
		proc_regs->r9 = regs->r9;
		proc_regs->r10 = regs->r10;
		proc_regs->r11 = regs->r11;
		proc_regs->r12 = regs->r12;
		proc_regs->r13 = regs->r13;
		proc_regs->r14 = regs->r14;
		proc_regs->r15 = regs->r15;
		proc_regs->cs = interrupt_frame->cs;
		proc_regs->rip = interrupt_frame->rip;
		proc_regs->rflags = interrupt_frame->rflags;
		proc_regs->ss = interrupt_frame->ss;

		processor->flags &= ~(1 << 1);
	}

	struct ARC_Registers *source = proc_regs;

	if (MASKED_READ(processor->flags, ARC_SMP_FLAGS_CTXWRITE, 1) == 1) {
	        ctx_switch:;
		struct ARC_Registers saved = { 0 };

		saved.rax = regs->rax;
		saved.rbx = regs->rbx;
		saved.rcx = regs->rcx;
		saved.rdx = regs->rdx;
		saved.rsi = regs->rsi;
		saved.rdi = regs->rdi;
		saved.rsp = interrupt_frame->rsp;
		saved.rbp = regs->rbp;
		saved.rip = regs->rip;
		saved.r8 = regs->r8;
		saved.r9 = regs->r9;
		saved.r10 = regs->r10;
		saved.r11 = regs->r11;
		saved.r12 = regs->r12;
		saved.r13 = regs->r13;
		saved.r14 = regs->r14;
		saved.r15 = regs->r15;
		saved.cs = interrupt_frame->cs;
		saved.rip = interrupt_frame->rip;
		saved.rflags = interrupt_frame->rflags;
		saved.ss = interrupt_frame->ss;

		// Switch to desired context
		regs->rax = source->rax;
		regs->rbx = source->rbx;
		regs->rcx = source->rcx;
		regs->rdx = source->rdx;
		regs->rsi = source->rsi;
		regs->rdi = source->rdi;
		interrupt_frame->rsp = source->rsp;
		regs->rbp = source->rbp;
		regs->rip = source->rip;
		regs->r8 = source->r8;
		regs->r9 = source->r9;
		regs->r10 = source->r10;
		regs->r11 = source->r11;
		regs->r12 = source->r12;
		regs->r13 = source->r13;
		regs->r14 = source->r14;
		regs->r15 = source->r15;
		interrupt_frame->cs = source->cs;
		interrupt_frame->rip = source->rip;
		interrupt_frame->rflags = source->rflags;
		interrupt_frame->ss = source->ss;

		smp_context_write(processor, &saved);
		processor->flags &= ~1;
	} else {
		struct ARC_Thread *thread= sched_tick();

		if (thread == NULL && (thread = sched_get_current_thread()) == NULL) {
			goto skip_threading;
		}


		if (processor->last_thread != NULL) {
			smp_context_save(processor, &processor->last_thread->ctx);
		}

		regs->cr3 = ARC_HHDM_TO_PHYS(processor->current_process->page_tables);
		processor->last_thread = processor->current_thread;
		processor->current_thread = thread;
		source = &thread->ctx;

		goto ctx_switch;
	}

	skip_threading:;

	mutex_unlock(&processor->register_lock);

	mutex_lock(&processor->timer_lock);

	if (processor->timer_mode == ARC_LAPIC_TIMER_ONESHOT) {
		lapic_refresh_timer(processor->timer_ticks);
	}
	if (MASKED_READ(processor->flags, ARC_SMP_FLAGS_WTIMER, 1) == 1) {
		lapic_setup_timer(32, processor->timer_mode);
		lapic_refresh_timer(processor->timer_ticks);

		processor->flags &= ~(1 << 2);
	}

	mutex_unlock(&processor->timer_lock);

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
