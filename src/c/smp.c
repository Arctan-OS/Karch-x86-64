/**
 * @file smp.c
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
 * This file implements functions for initializing and managing application processors
 * for symmetric multi-processing.
*/
#include "arch/acpi/table.h"
#include "arch/pager.h"
#include "arch/smp.h"
#include "arch/interrupt.h"
#include "arch/syscall.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/gdt.h"
#include "arch/x86-64/interrupt.h"
#include "arch/x86-64/smp.h"
#include "arch/x86-64/sse.h"
#include "arch/x86-64/util.h"
#include "lib/util.h"
#include "mm/allocator.h"
#include "mm/pmm.h"
#include "mp/scheduler.h"
#include "util.h"
#include "global.h"

enum {
        ARC_AP_INFO_FLAGS_LM = 0,       // The AP has reached LM
	ARC_AP_INFO_FLAGS_STARTED,      // The AP has started
	ARC_AP_INFO_FLAGS_PAT,          // AP should use provided PAT
	ARC_AP_INFO_FLAGS_NX,           // AP can use NX
};

typedef struct ARC_APStartInfo {
	uint64_t pml4;
	uint64_t entry;
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
	uint32_t acpi_uid;
	uint32_t acpi_flags;
}__attribute__((packed)) ARC_APStartInfo;

extern uint8_t _AP_START_BEGIN;
extern uint8_t _AP_START_END;
extern uint8_t _AP_START_INFO;

struct ARC_x64ProcessorDescriptor *Arc_ProcessorList = NULL;
struct ARC_x64ProcessorDescriptor *Arc_BootProcessor = NULL;
ARC_x64ProcessorDescriptor __seg_gs *Arc_CurProcessorDescriptor = NULL;
uint32_t Arc_ProcessorCounter = 0;

void smp_hold() {
	ARC_HANG;
}

static int smp_register_ap(uint32_t acpi_uid, uint32_t acpi_flags) {
 	init_lapic();

	ARC_x64ProcessorDescriptor *current = &Arc_ProcessorList[Arc_ProcessorCounter];
	context_set_proc_desc(current);

	if (Arc_ProcessorCounter == 0) {
		Arc_BootProcessor = current;
	}

	ARC_ProcessorDescriptor *desc = alloc(sizeof(*desc));
	if (desc == NULL) {
		ARC_DEBUG(ERR, "Failed to create processor descriptor\n");
		ARC_HANG;
	}

	memset(desc, 0, sizeof(*desc));

	current->descriptor = desc;

	desc->acpi_uid = acpi_uid;
	desc->acpi_flags = acpi_flags;

	uintptr_t ist1 = (uintptr_t)alloc(ARC_STD_KSTACK_SIZE);
	uintptr_t rsp0 = (uintptr_t)alloc(ARC_STD_KSTACK_SIZE);

	ARC_TSSDescriptor *tss = init_tss(ist1, rsp0);
	ARC_GDTRegister *gdtr = init_gdt();
	gdt_load(gdtr);
	gdt_use_tss(gdtr, tss);

	ARC_IDTRegister *idtr = init_dynamic_interrupts(256);
	internal_init_early_exceptions((ARC_IDTEntry *)idtr->base, 0x8, 1);
	interrupt_load(idtr);

	init_syscall();

	if (Arc_ProcessorCounter == 0) {
		lapic_setup_timer(32, ARC_LAPIC_TIMER_PERIODIC);
		desc->timer_ticks = 1000;
		desc->timer_mode = ARC_LAPIC_TIMER_PERIODIC;
		lapic_refresh_timer(1000);
		lapic_calibrate_timer();
	}

	interrupt_set(idtr, 32, ARC_NAME_IRQ(sched_timer_hook), true);

	init_sse();

	Arc_ProcessorCounter++;

	__asm__("swapgs");

	desc->flags |= 1 << ARC_SMP_FLAGS_INIT;

	return 0;
}

/**
 * Further initialize the application processor to synchronize with the BSP.
 *
 * NOTE: This function is only meant to be called by application processors.
 *
 * @param struct ap_start_info *info - The boot information given by the BSP.
 * */
static int smp_move_ap_high_mem(ARC_APStartInfo *info) {
	// NOTE: APs are initialized sequentially, therefore only one AP
	//       should be executing this code at a time
	smp_register_ap(info->acpi_uid, info->acpi_flags);

	ARC_DISABLE_INTERRUPT;
//	ARC_ENABLE_INTERRUPT;

	info->flags |= 1 << ARC_AP_INFO_FLAGS_LM;

	smp_hold();

	return 0;
}

struct ARC_ProcessorDescriptor *smp_get_proc_desc() {
	if (Arc_ProcessorCounter == 0) {
		// No processors have been initialized yet into an SMP system,
		// therefore it can be assumed that GS = 0. Accessing
		// CurProcessorDescriptor would lead to a PF, so return
		// NULL
		return NULL;
	}
	return Arc_CurProcessorDescriptor->descriptor;
}

uint32_t smp_get_processor_id() {
	return lapic_get_id();
}

void smp_switch_to(ARC_Context *ctx) {
	(void)ctx;
	ARC_DEBUG(WARN, "Definitely doing context switch\n");
}

// NOTE: This function is only called from the BSP
int smp_init_ap(uint32_t processor, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version) {
	if (processor == (uint32_t)lapic_get_id()) {
		smp_register_ap(acpi_uid, acpi_flags);
		return 0;
	}

	// Allocate space in low memory, copy ap_start code to it
	// which should bring AP to kernel_main where it will be
	// detected, logged, and put into smp_hold
	void *code = pmm_low_alloc(PAGE_SIZE * 2);
	void *stack = pmm_low_alloc(PAGE_SIZE * 2);

	// NOTE: This is a virtual address
	void *stack_high = alloc(PAGE_SIZE * 2);

	pager_map(NULL, ARC_HHDM_TO_PHYS(code), ARC_HHDM_TO_PHYS(code), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);
	pager_map(NULL, ARC_HHDM_TO_PHYS(stack), ARC_HHDM_TO_PHYS(stack), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);

	memset(code, 0, PAGE_SIZE);
	memcpy(code, (void *)&_AP_START_BEGIN, (size_t)((uintptr_t)&_AP_START_END - (uintptr_t)&_AP_START_BEGIN));
	ARC_APStartInfo *info = (ARC_APStartInfo *)((uintptr_t)code + ((uintptr_t)&_AP_START_INFO - (uintptr_t)&_AP_START_BEGIN));

	info->pml4 = _x86_getCR3();
	info->entry = (uintptr_t)smp_move_ap_high_mem;
	info->stack = ARC_HHDM_TO_PHYS(stack) + PAGE_SIZE - 0x8;
	info->stack_high = (uintptr_t)stack_high + (PAGE_SIZE * 2) - 0x8;
	info->gdt_size = 0x1F;
	info->gdt_addr = ARC_HHDM_TO_PHYS(&info->gdt_table);
	info->pat = _x86_RDMSR(0x277);
	info->flags |= (1 << ARC_AP_INFO_FLAGS_PAT);
	info->flags |= ((Arc_KernelMeta->paging_features & 1) << ARC_AP_INFO_FLAGS_NX);
	info->acpi_flags = acpi_flags;
	info->acpi_uid = acpi_uid;

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
	while (((info->flags >> ARC_AP_INFO_FLAGS_STARTED) & 1) == 0) __asm__("pause");

	ARC_DEBUG(INFO, "AP %d BIST: 0x%x\n", processor, info->eax);

	// TODO: If BIST indicates error, shut down AP, move on
	while (((info->flags >> ARC_AP_INFO_FLAGS_LM) & 1) == 0) __asm__("pause");

	pager_unmap(NULL, ARC_HHDM_TO_PHYS(code), PAGE_SIZE, NULL);
	pager_unmap(NULL, ARC_HHDM_TO_PHYS(stack), PAGE_SIZE, NULL);

	pmm_low_free(code);
	pmm_low_free(stack);

	return 0;
}

int init_smp() {
	ARC_MADTIterator it = NULL;

	size_t processors = 0;

	while (acpi_get_next_madt_entry(ARC_MADT_ENTRY_TYPE_LAPIC, &it) != NULL) {
		processors++;
	}

	Arc_ProcessorList = (ARC_x64ProcessorDescriptor *)alloc(sizeof(*Arc_ProcessorList) * processors);

	if (Arc_ProcessorList == NULL) {
		ARC_DEBUG(ERR, "Failed to initialize SMP\n");
		return -2;
	}

	return 0;
}
