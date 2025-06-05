/**
 * @file gdt.c
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
 * Change out the GDT to better suit 64-bit mode, remove no longer needed 32-bit
 * segments.
*/
#include <global.h>
#include <arch/x86-64/gdt.h>
#include <mm/allocator.h>
#include <lib/util.h>

struct gdt_header {
	uint16_t size;
	uint64_t base;
}__attribute__((packed));
struct gdt_header gdtr;

struct gdt_entry {
	uint16_t limit;
	uint16_t base1;
	uint8_t base2;
	uint8_t access;
	uint8_t flags_limit;
	uint8_t base3;
}__attribute__((packed));

struct tss_entry {
	uint16_t limit;
	uint16_t base1;
	uint8_t base2;
	uint8_t access;
	uint8_t flags_limit;
	uint8_t base3;
	uint32_t base4;
	uint32_t resv;
}__attribute__((packed));

struct tss_descriptor {
	uint32_t resv0;
	uint32_t rsp0_low;
	uint32_t rsp0_high;
	uint32_t rsp1_low;
	uint32_t rsp1_high;
	uint32_t rsp2_low;
	uint32_t rsp2_high;
	uint32_t resv1;
	uint32_t resv2;
	uint32_t ist1_low;
	uint32_t ist1_high;
	uint32_t ist2_low;
	uint32_t ist2_high;
	uint32_t ist3_low;
	uint32_t ist3_high;
	uint32_t ist4_low;
	uint32_t ist4_high;
	uint32_t ist5_low;
	uint32_t ist5_high;
	uint32_t ist6_low;
	uint32_t ist6_high;
	uint32_t ist7_low;
	uint32_t ist7_high;
	uint32_t resv3;
	uint32_t resv4;
	uint16_t resv5;
	uint16_t io_port_bmp_off;
}__attribute__((packed));

struct gdt_entry_container {
	struct gdt_entry gdt[8];
	struct tss_entry tss;
}__attribute__((packed));

// TODO: Is this really the best way?
static uint8_t ists[ARC_MAX_PROCESSORS][PAGE_SIZE] __attribute__((aligned(PAGE_SIZE))) = { 0 };
static uint8_t rsps[ARC_MAX_PROCESSORS][PAGE_SIZE] __attribute__((aligned(PAGE_SIZE))) = { 0 };
static struct gdt_entry_container gdts[ARC_MAX_PROCESSORS] = { 0 };
static struct tss_descriptor tsss[ARC_MAX_PROCESSORS] = { 0 };

void set_gdt_gate(struct gdt_entry_container *gdt_entries, int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries->gdt[i].base1 = (base      ) & 0xFFFF;
	gdt_entries->gdt[i].base2 = (base >> 16) & 0xFF;
	gdt_entries->gdt[i].base3 = (base >> 24) & 0xFF;

	gdt_entries->gdt[i].access = access;

	gdt_entries->gdt[i].limit = (limit) & 0xFFFF;
	gdt_entries->gdt[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

void set_tss_gate(struct gdt_entry_container *gdt_entries, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries->tss.base1 = (base      ) & 0xFFFF;
	gdt_entries->tss.base2 = (base >> 16) & 0xFF;
	gdt_entries->tss.base3 = (base >> 24) & 0xFF;
	gdt_entries->tss.base4 = (base >> 32) & UINT32_MAX;

	gdt_entries->tss.access = access;

	gdt_entries->tss.limit = (limit) & 0xFFFF;
	gdt_entries->tss.flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

extern void _install_gdt();
extern void _install_tss(uint32_t index);
uintptr_t init_gdt(int processor) {
	ARC_DEBUG(INFO, "Initializing GDT\n");

	struct gdt_entry_container *container = &gdts[processor];

	set_gdt_gate(container, 0, 0, 0, 0, 0);
	set_gdt_gate(container, 1, 0, 0xFFFFFFFF, 0x9A, 0xA); // Kernel Code 64
	set_gdt_gate(container, 2, 0, 0xFFFFFFFF, 0x92, 0xC); // Kernel Data 32 / 64
	set_gdt_gate(container, 3, 0, 0xFFFFFFFF, 0xF2, 0xC); // User Data 32 / 64
	set_gdt_gate(container, 4, 0, 0xFFFFFFFF, 0xFA, 0xA); // User Code 64

	ARC_DEBUG(INFO, "Installed basic descriptors\n");

	struct tss_descriptor *tss = &tsss[processor];

	set_tss_gate(container, (uint64_t)tss, sizeof(*tss) - 1, 0x89, 0x0);

	uintptr_t ist = (uintptr_t)&ists[processor] + PAGE_SIZE - 8;
	uintptr_t rsp = (uintptr_t)&rsps[processor] + PAGE_SIZE - 8;
	tss->ist1_low = (ist & UINT32_MAX);
	tss->ist1_high = (ist >> 32) & UINT32_MAX;
	tss->rsp0_low = (rsp & UINT32_MAX);
	tss->rsp0_high = (rsp >> 32) & UINT32_MAX;

	ARC_DEBUG(INFO, "Created TSS\n");

	gdtr.size = sizeof(*container) - 1;
	gdtr.base = (uintptr_t)container;

	_install_gdt();
	ARC_DEBUG(INFO, "Installed GDT\n");

	_install_tss(sizeof(container->gdt));
	ARC_DEBUG(INFO, "Installed TSS\n");
	ARC_DEBUG(INFO, "Initialized GDT\n");

	return rsp;
}
