/**
 * @file gdt.c
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
 * Change out the GDT to better suit 64-bit mode, remove no longer needed 32-bit
 * segments.
*/
#include "arch/x86-64/gdt.h"
#include "lib/util.h"
#include "mm/allocator.h"
#include "global.h"

void set_gdt_gate(ARC_GDTRegister *gdtr, int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	if (i >= 8) {
		ARC_DEBUG(ERR, "Cannot access GDT entry %d, out of bounds\n", i);
		return;
	}

	gdtr->gdt[i].base1 = (base      ) & 0xFFFF;
	gdtr->gdt[i].base2 = (base >> 16) & 0xFF;
	gdtr->gdt[i].base3 = (base >> 24) & 0xFF;

	gdtr->gdt[i].access = access;

	gdtr->gdt[i].limit = (limit) & 0xFFFF;
	gdtr->gdt[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

void set_tss_gate(ARC_GDTRegister *gdtr, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdtr->tss.base1 = (base      ) & 0xFFFF;
	gdtr->tss.base2 = (base >> 16) & 0xFF;
	gdtr->tss.base3 = (base >> 24) & 0xFF;
	gdtr->tss.base4 = (base >> 32) & UINT32_MAX;

	gdtr->tss.access = access;

	gdtr->tss.limit = (limit) & 0xFFFF;
	gdtr->tss.flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

extern int _install_gdt(void *gdtr);
int gdt_load(ARC_GDTRegister *gdtr) {
	return _install_gdt(&gdtr->reg);
}

extern int _install_tss(uint32_t index);
int gdt_use_tss(ARC_GDTRegister *gdtr, ARC_TSSDescriptor *tss) {
	set_tss_gate(gdtr, (uintptr_t)tss, sizeof(*tss) - 1, 0x89, 0x0);
	return _install_tss(sizeof(gdtr->gdt));
}

ARC_TSSDescriptor *init_tss(uintptr_t ist1, uintptr_t rsp0) {
	ARC_TSSDescriptor *tss = alloc(sizeof(*tss));

	if (tss == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate TSS\n");
		return NULL;
	}

	memset(tss, 0, sizeof(*tss));

	tss->ist1_low = (ist1 & UINT32_MAX);
	tss->ist1_high = (ist1 >> 32) & UINT32_MAX;
	tss->rsp0_low = (rsp0 & UINT32_MAX);
	tss->rsp0_high = (rsp0 >> 32) & UINT32_MAX;

	ARC_DEBUG(INFO, "Created TSS\n");

	return tss;
}

ARC_GDTRegister *init_gdt() {
	ARC_DEBUG(INFO, "Initializing GDT\n");

	ARC_GDTRegister *gdtr = alloc(sizeof(*gdtr));

	if (gdtr == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate GDTR\n");
		return NULL;
	}

	memset(gdtr, 0, sizeof(*gdtr));

	set_gdt_gate(gdtr, 1, 0, 0xFFFFFFFF, 0x9A, 0xA); // Kernel Code 64
	set_gdt_gate(gdtr, 2, 0, 0xFFFFFFFF, 0x92, 0xC); // Kernel Data 32 / 64
	set_gdt_gate(gdtr, 3, 0, 0xFFFFFFFF, 0xF2, 0xC); // User Data 32 / 64
	set_gdt_gate(gdtr, 4, 0, 0xFFFFFFFF, 0xFA, 0xA); // User Code 64

	gdtr->reg.size = sizeof(*gdtr) - 1 - sizeof(gdtr->reg);
	gdtr->reg.base = (uintptr_t)gdtr;

	return gdtr;
}
