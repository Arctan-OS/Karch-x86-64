/**
 * @file smp.c
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
 * This file implements functions for initializing and managing application processors
 * for symmetric multi-processing.
*/
#include <arch/x86-64/apic/lapic.h>
#include <interface/printf.h>
#include <global.h>
#include <arch/pager.h>
#include <mm/pmm.h>
#include <lib/util.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/allocator.h>
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/smp.h>
#include <arch/start.h>
#include <arch/x86-64/syscall.h>

struct ap_start_info {
	uint64_t pml4;
	uint64_t entry;
	// Flags
	//  Bit | Description
	//  0   | LM reached, core jumping to kernel_entry
	//  1   | Processor core started
	//  2   | Use PAT given
	//  3   | NX
	uint32_t flags;
	uint16_t gdt_size;
	uint64_t gdt_addr;
	uint64_t gdt_table[4];
	uint64_t stack;
	// EDX: Processor information
	uint32_t edx;
	// EAX: Return value of BIST
	uint32_t eax;
	uint64_t pat;
	uint64_t stack_high;
}__attribute__((packed));

extern uint8_t __AP_START_BEGIN__;
extern uint8_t __AP_START_END__;
extern uint8_t __AP_START_INFO__;

struct ARC_ProcessorDescriptor Arc_ProcessorList[ARC_MAX_PROCESSORS] = { 0 };
uint32_t Arc_ProcessorCounter = 0;
struct ARC_ProcessorDescriptor *Arc_BootProcessor = NULL;

static uint32_t last_lapic = 0;
static uint8_t *userspace_hold = NULL;
static ARC_GenericSpinlock userspace_lock = 0;

/**
 * Further initialize the application processor to synchronize with the BSP.
 *
 * NOTE: This function is only meant to be called by application processors.
 *
 * @param struct ap_start_info *info - The boot information given by the BSP.
 * */
int smp_move_ap_high_mem(struct ap_start_info *info) {
	// NOTE: APs are initialized sequentially, therefore only one AP
	//       should be executing this code at a time
	int id = lapic_get_id();

	if (id == -1) {
		ARC_DEBUG(ERR, "This is impossible, how can you have LAPIC but no LAPIC?\n");
		ARC_HANG;
	}

	Arc_ProcessorList[id].syscall_stack = init_gdt(id);
	_install_idt();

	init_lapic();
	lapic_setup_timer(32, ARC_LAPIC_TIMER_PERIODIC);
	Arc_ProcessorList[id].timer_ticks = 1000;
	Arc_ProcessorList[id].timer_mode = ARC_LAPIC_TIMER_PERIODIC;
	lapic_refresh_timer(1000);
	lapic_calibrate_timer();

	_x86_WRMSR(0xC0000102, (uintptr_t)&Arc_ProcessorList[id]);

	init_syscall();

	Arc_ProcessorList[id].flags |= 1 << ARC_SMP_FLAGS_INIT;

	__asm__("sti");

	info->flags |= 1;

	smp_hold(&Arc_ProcessorList[id]);

	return 0;
}

int smp_hold(struct ARC_ProcessorDescriptor *processor) {
	processor->flags |= 1 << ARC_SMP_FLAGS_HOLD;
	ARC_HANG;
}

int smp_context_write(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs) {
	if (processor == NULL || regs == NULL) {
		return 1;
	}

	processor->registers.rax = regs->rax;
	processor->registers.rbx = regs->rbx;
	processor->registers.rcx = regs->rcx;
	processor->registers.rdx = regs->rdx;
	processor->registers.rsi = regs->rsi;
	processor->registers.rdi = regs->rdi;
	processor->registers.rsp = regs->rsp;
	processor->registers.rbp = regs->rbp;
	processor->registers.r8 = regs->r8;
	processor->registers.r9 = regs->r9;
	processor->registers.r10 = regs->r10;
	processor->registers.r11 = regs->r11;
	processor->registers.r12 = regs->r12;
	processor->registers.r13 = regs->r13;
	processor->registers.r14 = regs->r14;
	processor->registers.r15 = regs->r15;
	processor->registers.cs = regs->cs;
	processor->registers.rip = regs->rip;
	processor->registers.rflags = regs->rflags;
	processor->registers.ss = regs->ss;

	processor->flags |= 1 << ARC_SMP_FLAGS_CTXWRITE;

	return 0;
}

int smp_context_save(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs) {
	if (processor == NULL || regs == NULL) {
		return 1;
	}

	regs->rax = processor->registers.rax;
	regs->rbx = processor->registers.rbx;
	regs->rcx = processor->registers.rcx;
	regs->rdx = processor->registers.rdx;
	regs->rsi = processor->registers.rsi;
	regs->rdi = processor->registers.rdi;
	regs->rsp = processor->registers.rsp;
	regs->rbp = processor->registers.rbp;
	regs->rip = processor->registers.rip;
	regs->r8 = processor->registers.r8;
	regs->r9 = processor->registers.r9;
	regs->r10 = processor->registers.r10;
	regs->r11 = processor->registers.r11;
	regs->r12 = processor->registers.r12;
	regs->r13 = processor->registers.r13;
	regs->r14 = processor->registers.r14;
	regs->r15 = processor->registers.r15;
	regs->cs = processor->registers.cs;
	regs->rip = processor->registers.rip;
	regs->rflags = processor->registers.rflags;
	regs->ss = processor->registers.ss;

	return 0;
}

/**
 * Pass given arguments to given processor.
 *
 * Set registers and stack according to SYS-V calling convention.
 *
 * NOTE: It is expected for processor->register_lock to be held.
 * */
int smp_sysv_set_args(struct ARC_ProcessorDescriptor *processor, va_list list, int argc) {
	int i = 0;

	for (; i < min(argc, 6); i++) {
		uint64_t value = va_arg(list, uint64_t);

		switch (i) {
			case 0: { processor->registers.rdi = value; break; }
			case 1: { processor->registers.rsi = value; break; }
			case 2: { processor->registers.rdx = value; break; }
			case 3: { processor->registers.rcx = value; break; }
			case 4: { processor->registers.r8  = value; break; }
			case 5: { processor->registers.r9  = value; break; }
		}
	}

	if (argc <= 6) {
		return 0;
	}

	int delta = argc - i;

	for (i = delta - 1; i >= 0; i--) {
		uint64_t value = va_arg(list, uint64_t);
		*(uint64_t *)(processor->registers.rsp - (i * 8)) = value;
	}

	processor->registers.rsp -= delta * 8;

	return 0;
}

int smp_jmp(struct ARC_ProcessorDescriptor *processor, void *function, int argc, ...) {
	mutex_lock(&processor->register_lock);

	va_list args;
	va_start(args, argc);
	smp_sysv_set_args(processor, args, argc);
	va_end(args);

	processor->registers.rip = (uintptr_t)function;

	mutex_unlock(&processor->register_lock);

	processor->flags &= ~(1 << ARC_SMP_FLAGS_HOLD);
	processor->flags |= 1 << ARC_SMP_FLAGS_CTXWRITE;

	return 0;
}

struct ARC_ProcessorDescriptor *smp_get_proc_desc() {
	struct ARC_ProcessorDescriptor *result = NULL;

	__asm__("swapgs");
	result = (struct ARC_ProcessorDescriptor *)_x86_RDMSR(0xC0000101);
	__asm__("swapgs");

	return result;
}

uint32_t smp_get_processor_id() {
	return lapic_get_id();
}

int smp_switch_to_userspace() {
	// NOTE: For other architectures this may be useful, however on x86-64, the timer
	//       interrupt can do this work by setting the return SS and CS to be of ones
	//       that have a privelege level of 3. This function is kept as a way for the
	//       kernel to explicitly state that all processors are to be switched to userspace
	return 0;
}

int init_smp(uint32_t processor, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version) {
	// NOTE: This function is only called from the BSP
	Arc_ProcessorList[processor].acpi_uid = acpi_uid;
	Arc_ProcessorList[processor].acpi_flags = acpi_flags;
	Arc_ProcessorList[last_lapic].next = &Arc_ProcessorList[processor];
	last_lapic = processor;

	if (processor == (uint32_t)lapic_get_id()) {
		// BSP
		Arc_ProcessorList[processor].syscall_stack = init_gdt(processor);
		init_idt();
		init_syscall();

		Arc_BootProcessor = &Arc_ProcessorList[processor];
		Arc_BootProcessor->flags |= 1 << ARC_SMP_FLAGS_INIT;
		Arc_ProcessorCounter++;
		_x86_WRMSR(0xC0000102, (uintptr_t)Arc_BootProcessor);

		return 0;
	}

	// Set warm reset in CMOS and warm reset
        // vector in BDA (not sure if this is needed)
        // cmos_write(0xF, 0xA);
        // *(uint32_t *)(ARC_PHYS_TO_HHDM(0x467)) = (uint32_t)((uintptr_t)code);

	// Allocate space in low memory, copy ap_start code to it
	// which should bring AP to kernel_main where it will be
	// detected, logged, and put into smp_hold
	void *code = pmm_low_alloc();
	void *stack = pmm_low_alloc();

	// NOTE: This is a virtual address
	void *stack_high = alloc(PAGE_SIZE * 2);

	pager_map(NULL, ARC_HHDM_TO_PHYS(code), ARC_HHDM_TO_PHYS(code), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);
	pager_map(NULL, ARC_HHDM_TO_PHYS(stack), ARC_HHDM_TO_PHYS(stack), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);

	memset(code, 0, PAGE_SIZE);
	memcpy(code, (void *)&__AP_START_BEGIN__, (size_t)((uintptr_t)&__AP_START_END__ - (uintptr_t)&__AP_START_BEGIN__));
	struct ap_start_info *info = (struct ap_start_info *)((uintptr_t)code + ((uintptr_t)&__AP_START_INFO__ - (uintptr_t)&__AP_START_BEGIN__));

	_x86_getCR3();
	info->pml4 = _x86_CR3;
	info->entry = (uintptr_t)smp_move_ap_high_mem;
	info->stack = ARC_HHDM_TO_PHYS(stack) + PAGE_SIZE - 0x8;
	info->stack_high = (uintptr_t)stack_high + (PAGE_SIZE * 2) - 0x8;
	info->gdt_size = 0x1F;
	info->gdt_addr = ARC_HHDM_TO_PHYS(&info->gdt_table);
	info->pat = _x86_RDMSR(0x277);
	info->flags |= (1 << 2);
	info->flags |= ((Arc_BootMeta->paging_features & 1) << 3);

	ARC_MEM_BARRIER;

	// AP start procedure
	// INIT IPI
	lapic_ipi(0, processor, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_ASSERT);
	while (lapic_ipi_poll()) __asm__("pause");
	// INIT De-assert IPI
	lapic_ipi(0, processor, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_DEASRT);
	while (lapic_ipi_poll()) __asm__("pause");

	// If (lapic->version != 82489DX)
	if (version >= 0xA) {
		uint8_t vector = (((uintptr_t)code) >> 12) & 0xFF;

		// SIPI
		lapic_ipi(vector, processor, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
		// SIPI
		lapic_ipi(vector, processor, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
	}

	// TODO: If this flag is not set within a reasonable time, skip this processor
	while (((info->flags >> 1) & 1) == 0) __asm__("pause");

	ARC_DEBUG(INFO, "AP %d BIST: 0x%x\n", processor, info->eax);

	// TODO: If BIST indicates error, shut down AP, move on
	while ((info->flags & 1) == 0) __asm__("pause");

	pager_unmap(NULL, ARC_HHDM_TO_PHYS(code), PAGE_SIZE);
	pager_unmap(NULL, ARC_HHDM_TO_PHYS(stack), PAGE_SIZE);

	pmm_low_free(code);
	pmm_low_free(stack);

	Arc_ProcessorCounter++;

	return 0;
}
