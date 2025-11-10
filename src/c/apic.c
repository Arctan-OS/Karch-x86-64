/**
 * @file apic.c
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
#include "arch/acpi/acpi.h"
#include "arch/acpi/table.h"
#include "arch/interrupt.h"
#include "arch/io/port.h"
#include "arch/x86-64/apic.h"
#include "arch/x86-64/apic/io.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/smp.h"
#include "arch/x86-64/util.h"
#include "global.h"
#include "mm/allocator.h"

typedef struct ARC_IOAPICElement {
	struct ARC_IOAPICElement *next;
	ARC_IOAPICReg *ioapic;
	uint32_t gsi;
	uint32_t mre;
	uint32_t id;
} ARC_IOAPICElement;

ARC_IOAPICElement *ioapic_list = NULL;

int interrupts_map_gsi(uint32_t gsi, uint32_t to_irq, uint32_t to_id, uint8_t flags) {
	// Flags (bitwise)
	//     Offset | Description
	//     0      | 1: Level Triggered 0: Edge Triggered
	//     1      | 1: Active Low      0: Active High
	//     2      | 1: destination     0: destination refers
	//                 refers             to a single LAPIC
        //                 to a group of
	//                 LAPICs

	ARC_IOAPICElement *current = ioapic_list;
	while (current != NULL) {
		if (gsi >= current->gsi && gsi <= current->gsi + current->mre) {
			break;
		}

		current = current->next;
	}

	ARC_IOAPICRedirTable table = {
	        .trigger = (flags & 1),
		.int_pol = ((flags >> 1) & 1),
		.mask = 0,
		.dest_mod = ((flags >> 2) & 1),
		.destination = to_id,
		.int_vec = (to_irq + 32),
        };

	ioapic_write_redir_tbl(current->ioapic, (gsi - current->gsi), &table);

	return 0;
}

int init_apic() {
	ARC_MADTIterator it = NULL;
	ARC_MADTLapic *lapic = NULL;
	while ((lapic = acpi_get_next_madt_entry(ARC_MADT_ENTRY_TYPE_LAPIC, &it)) != NULL) {
		uint32_t uid = lapic->uid;
		uint32_t id = lapic->id;
		uint32_t flags = lapic->flags;

		ARC_DEBUG(INFO, "LAPIC found (UID: %d, ID: %d, Flags: 0x%x)\n", uid, id, flags);

		smp_init_ap(id, uid, flags, 0xFF);
	}

	it = NULL;
	ARC_MADTIOApic *ioapic = NULL;
	while ((ioapic = acpi_get_next_madt_entry(ARC_MADT_ENTRY_TYPE_IOAPIC, &it)) != NULL) {
		uint32_t address = ioapic->address;
		uint32_t gsi = ioapic->gsi;
		uint32_t id = ioapic->id;

		int mre = init_ioapic(address);

		ARC_IOAPICElement *next = (ARC_IOAPICElement *)alloc(sizeof(*next));

		if (next == NULL) {
			ARC_DEBUG(ERR, "\tFailed to allocate IOAPIC descriptor\n");
			break;
		}

		next->gsi = gsi;
		next->mre = mre;
		next->ioapic = (ARC_IOAPICReg *)((uintptr_t)address);
		next->id = id;

		ARC_DEBUG(INFO, "IOAPIC found (GSI: %d, MRE: %d, Address: 0x%"PRIx32", ID: %d)\n", gsi, mre, address, id);

		next->next = ioapic_list;
		ioapic_list = next;
	}

	if (ioapic_list == NULL) {
		// Disable 8259
		outb(0x21, 0xFF);
		outb(0xA1, 0xFF);
	}

	return 0;
}
