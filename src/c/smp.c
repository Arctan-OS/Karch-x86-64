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
#include <arch/x86-64/smp.h>
#include <arch/x86-64/apic/lapic.h>
#include <interface/printf.h>
#include <global.h>
#include <arch/x86-64/pager.h>
#include <mm/pmm.h>
#include <lib/util.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/allocator.h>
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/sse.h>
#include <arch/smp.h>

extern uint8_t __AP_START_BEGIN__;
extern uint8_t __AP_START_END__;
extern uint8_t __AP_START_INFO__;

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

static uint32_t last_lapic = 0;

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

	init_gdt();
	_install_idt();

	init_lapic();
	lapic_setup_timer(32, ARC_LAPIC_TIMER_PERIODIC);
	Arc_ProcessorList[id].generic.timer_ticks = 1000;
	Arc_ProcessorList[id].generic.timer_mode = ARC_LAPIC_TIMER_PERIODIC;
	lapic_refresh_timer(1000);
	lapic_calibrate_timer();

	Arc_ProcessorList[id].generic.status |= 1;

	__asm__("sti");

	info->flags |= 1;

	smp_hold(&Arc_ProcessorList[id]);

	return 0;
}

int smp_hold(struct ARC_ProcessorDescriptor *processor) {
	processor->generic.status |= 1 << 1;
	ARC_DEBUG(INFO, "Holding processor %d\n", processor->generic.processor);
	ARC_HANG;
}

int smp_context_write(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs) {
	if (processor == NULL || regs == NULL) {
		return 1;
	}

	processor->generic.registers.rax = regs->rax;
	processor->generic.registers.rbx = regs->rbx;
	processor->generic.registers.rcx = regs->rcx;
	processor->generic.registers.rdx = regs->rdx;
	processor->generic.registers.rsi = regs->rsi;
	processor->generic.registers.rdi = regs->rdi;
	processor->generic.registers.rsp = regs->rsp;
	processor->generic.registers.rbp = regs->rbp;
	processor->generic.registers.r8 = regs->r8;
	processor->generic.registers.r9 = regs->r9;
	processor->generic.registers.r10 = regs->r10;
	processor->generic.registers.r11 = regs->r11;
	processor->generic.registers.r12 = regs->r12;
	processor->generic.registers.r13 = regs->r13;
	processor->generic.registers.r14 = regs->r14;
	processor->generic.registers.r15 = regs->r15;
	processor->generic.registers.cs = regs->cs;
	processor->generic.registers.rip = regs->rip;
	processor->generic.registers.rflags = regs->rflags;
	processor->generic.registers.ss = regs->ss;

	processor->generic.flags |= 1;

	return 0;
}

int smp_context_save(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs) {
	if (processor == NULL || regs == NULL) {
		return 1;
	}

	regs->rax = processor->generic.registers.rax;
	regs->rbx = processor->generic.registers.rbx;
	regs->rcx = processor->generic.registers.rcx;
	regs->rdx = processor->generic.registers.rdx;
	regs->rsi = processor->generic.registers.rsi;
	regs->rdi = processor->generic.registers.rdi;
	regs->rsp = processor->generic.registers.rsp;
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
	regs->cs = processor->generic.registers.cs;
	regs->rip = processor->generic.registers.rip;
	regs->rflags = processor->generic.registers.rflags;
	regs->ss = processor->generic.registers.ss;

	return 0;
}

/**
 * Pass given arguments to given processor.
 *
 * Set registers and stack according to SYS-V calling convention.
 *
 * NOTE: It is expected for processor->generic.register_lock to be held.
 * */
int smp_sysv_set_args(struct ARC_ProcessorDescriptor *processor, va_list list, int argc) {
	int i = 0;

	for (; i < min(argc, 6); i++) {
		uint64_t value = va_arg(list, uint64_t);

		switch (i) {
			case 0: { processor->generic.registers.rdi = value; break; }
			case 1: { processor->generic.registers.rsi = value; break; }
			case 2: { processor->generic.registers.rdx = value; break; }
			case 3: { processor->generic.registers.rcx = value; break; }
			case 4: { processor->generic.registers.r8  = value; break; }
			case 5: { processor->generic.registers.r9  = value; break; }
		}
	}

	if (argc <= 6) {
		return 0;
	}

	int delta = argc - i;

	for (i = delta - 1; i >= 0; i--) {
		uint64_t value = va_arg(list, uint64_t);
		*(uint64_t *)(processor->generic.registers.rsp - (i * 8)) = value;
	}

	processor->generic.registers.rsp -= delta * 8;

	return 0;
}

int smp_jmp(struct ARC_ProcessorDescriptor *processor, void *function, int argc, ...) {
	mutex_lock(&processor->generic.register_lock);

	va_list args;
	va_start(args, argc);
	smp_sysv_set_args(processor, args, argc);
	va_end(args);

	processor->generic.registers.rip = (uintptr_t)function;

	mutex_unlock(&processor->generic.register_lock);

	processor->generic.status &= ~(1 << 1);
	processor->generic.flags |= 1;

	return 0;
}

int smp_far_jmp(struct ARC_ProcessorDescriptor *processor, uint32_t cs, void *function, int argc, ...) {
	mutex_lock(&processor->generic.register_lock);

	va_list args;
	va_start(args, argc);
	smp_sysv_set_args(processor, args, argc);
	va_end(args);

	processor->generic.registers.cs = cs;
	processor->generic.registers.rip = (uintptr_t)function;

	mutex_unlock(&processor->generic.register_lock);

	processor->generic.status &= ~(1 << 1);
	processor->generic.flags |= 1;

	return 0;
}

int smp_list_aps() {
	printf("-- %d Processors --\n", Arc_ProcessorCounter);
	struct ARC_ProcessorDescriptor *current = Arc_BootProcessor;

	while (current != NULL) {
		printf("-------------------------\nProcessor #%d:\n\tSTATUS: 0x%x\n\tBIST:  0x%x\n\tModel: 0x%x\n-------------------------\n", current->generic.processor, current->generic.status, current->bist, current->model_info);
		current = current->generic.next;
	}

	return 0;
}

int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version) {
	// NOTE: This function is only called from the BSP
	Arc_ProcessorList[lapic].generic.processor = Arc_ProcessorCounter;
	Arc_ProcessorList[lapic].generic.acpi_uid = acpi_uid;
	Arc_ProcessorList[lapic].generic.acpi_flags = acpi_flags;
	Arc_ProcessorList[lapic].bist = 0x0;
	Arc_ProcessorList[lapic].model_info = 0x0; // TODO: Set this properly
	Arc_ProcessorList[last_lapic].generic.next = &Arc_ProcessorList[lapic];
	last_lapic = lapic;

	if (lapic == (uint32_t)lapic_get_id()) {
		// BSP
		Arc_BootProcessor = &Arc_ProcessorList[lapic];
		Arc_BootProcessor->generic.status |= 1;

		Arc_ProcessorCounter++;

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

	pager_map(ARC_HHDM_TO_PHYS(code), ARC_HHDM_TO_PHYS(code), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);
	pager_map(ARC_HHDM_TO_PHYS(stack), ARC_HHDM_TO_PHYS(stack), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);

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
	lapic_ipi(0, lapic, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_ASSERT);
	while (lapic_ipi_poll()) __asm__("pause");
	// INIT De-assert IPI
	lapic_ipi(0, lapic, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_DEASRT);
	while (lapic_ipi_poll()) __asm__("pause");

	// If (lapic->version != 82489DX)
	if (version >= 0xA) {
		uint8_t vector = (((uintptr_t)code) >> 12) & 0xFF;

		// SIPI
		lapic_ipi(vector, lapic, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
		// SIPI
		lapic_ipi(vector, lapic, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
	}

	while (((info->flags >> 1) & 1) == 0) __asm__("pause");

	Arc_ProcessorList[lapic].bist = info->eax;
	Arc_ProcessorList[lapic].model_info = info->edx;

	while ((info->flags & 1) == 0) __asm__("pause");

	pager_unmap(ARC_HHDM_TO_PHYS(code), PAGE_SIZE);
	pager_unmap(ARC_HHDM_TO_PHYS(stack), PAGE_SIZE);

	pmm_low_free(code);
	pmm_low_free(stack);

	Arc_ProcessorCounter++;

	return 0;
}
