/**
 * @file interrupt.c
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
#include "arch/interrupt.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/interrupt.h"
#include "arch/x86-64/context.h"
#include "arch/x86-64/util.h"
#include "lib/util.h"
#include "mm/allocator.h"
#include "util.h"

#define KERNEL_CS 0x08
#define USER_CS 0x18

void install_idt_gate(ARC_IDTEntry *entry, uint64_t offset, uint16_t segment, uint8_t attrs, int ist) {
	entry->offset1 = offset & 0xFFFF;
	entry->offset2 = (offset >> 16) & 0xFFFF;
	entry->offset3 = (offset >> 32) & 0xFFFFFFFF;

	entry->segment = segment;
	entry->attrs = attrs;
	entry->ist = ist;
	entry->reserved = 0;
}

// What if multiple functions could be registered to an interrupt?
int interrupt_set(void *handle, uint32_t number, void (*function)(ARC_InterruptFrame *), bool kernel) {
	if (number >= 256) {
		return -1;
	}

	ARC_IDTRegister *reg = (ARC_IDTRegister *)handle;
	ARC_IDTEntry *entries = NULL;

	if (reg == NULL) {
		ARC_IDTRegister t = { 0 };
		__asm__("sidt %0" :: "m"(t) :);
		reg = &t;
	}

	entries = (ARC_IDTEntry *)reg->base;

	ARC_DISABLE_INTERRUPT;

	if (function == NULL) {
		memset(&entries[number], 0, sizeof(ARC_IDTEntry));
	} else {
		install_idt_gate(&entries[number], (uintptr_t)function, kernel ? KERNEL_CS : USER_CS, 0x8E, 1 - !kernel);
	}

	// TODO: Make a way to save RFLAGS prior to disabling interrupts and
	//       then restore those values here

	return 0;
}

extern int _install_idt(void *);
int interrupt_load(void *handle) {
	return _install_idt(handle);
}

void interrupt_end() {
	lapic_eoi();
}

void *init_dynamic_interrupts(int count) {
	if (count < 32) {
		return NULL;
	}

	ARC_IDTRegister *reg = alloc(sizeof(*reg));

	if (reg == NULL) {
		return NULL;
	}

	ARC_IDTEntry *entries = alloc(sizeof(ARC_IDTEntry) * count);

	if (entries == NULL) {
		free(reg);
		return NULL;
	}

	memset(entries, 0, sizeof(ARC_IDTEntry) * count);

	reg->base = (uintptr_t)entries;
	reg->limit = sizeof(ARC_IDTEntry) * count - 1;

	return reg;
}

