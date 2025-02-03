/**
 * @file apic.c
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
*/
#include <arch/x86-64/apic/apic.h>
#include <arch/x86-64/apic/lapic.h>
#include <arch/x86-64/apic/ioapic.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <global.h>
#include <arch/smp.h>
#include <lib/util.h>
#include <lib/perms.h>
#include <arch/acpi/acpi.h>
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/smp.h>

#define ENTRY_TYPE_LAPIC               0x00
#define ENTRY_TYPE_IOAPIC              0x01
#define ENTRY_TYPE_INT_OVERRIDE_SRC    0x02
#define ENTRY_TYPE_NMI_SOURCE          0x03
#define ENTRY_TYPE_LAPIC_NMI           0x04
#define ENTRY_TYPE_LAPIC_ADDR_OVERRIDE 0x05
#define ENTRY_TYPE_IOSAPIC             0x06
#define ENTRY_TYPE_LSAPIC              0x07
#define ENTRY_TYPE_PIS                 0x08
#define ENTRY_TYPE_Lx2APIC             0x09
#define ENTRY_TYPE_Lx2APIC_NMI         0x0A
#define ENTRY_TYPE_GICC                0x0B
#define ENTRY_TYPE_GICD                0x0C
#define ENTRY_TYPE_GIC_MSI             0x0D
#define ENTRY_TYPE_GICR                0x0E
#define ENTRY_TYPE_ITS                 0x0F
#define ENTRY_TYPE_MP_WAKEUP           0x10

struct ioapic_element {
	uint32_t gsi;
	uint32_t mre;
	uint32_t id;
	struct ioapic_register *ioapic;
	struct ioapic_element *next;
};

struct ioapic_element *ioapic_list = NULL;

int apic_map_gsi_irq(uint8_t gsi, uint8_t irq, uint32_t destination, uint32_t flags) {
	// Flags (bitwise)
	//     Offset | Description
	//     0      | 1: Level Triggered 0: Edge Triggered
	//     1      | 1: Active Low      0: Active High
	//     2      | 1: destination     0: destination refers
	//                 refers             to a single LAPIC
        //                 to a group of
	//                 LAPICs

	struct ioapic_element *current = ioapic_list;
	while (current != NULL) {
		if (gsi >= current->gsi && gsi <= current->gsi + current->mre) {
			break;
		}

		current = current->next;
	}

	struct ioapic_redir_tbl table = {
	        .trigger = (flags & 1),
		.int_pol = ((flags >> 1) & 1),
		.mask = 0,
		.dest_mod = ((flags >> 2) & 1),
		.destination = destination,
		.int_vec = (irq + 32),
        };

	ioapic_write_redir_tbl(current->ioapic, (gsi - current->gsi), &table);

	return 0;
}

int init_apic() {
	uint32_t bsp = init_lapic();

	lapic_setup_timer(32, ARC_LAPIC_TIMER_PERIODIC);
	Arc_ProcessorList[bsp].timer_ticks = 1000;
	Arc_ProcessorList[bsp].timer_mode = ARC_LAPIC_TIMER_PERIODIC;
	lapic_refresh_timer(1000);

	uint8_t *data = NULL;
	size_t max = acpi_get_table("APIC", &data);
	size_t i = 0;

	if (data == NULL || max == 0) {
		return -1;
	}

	// NOTE: Is it possible for this data to be structured something
	//       along the lines of:
	//         - LAPIC
	//         - LAPIC
	//         - Interrupt Source Override
	//         - Interrupt Source Override
	//         - IOAPIC
	//         - ...
	//         Because if so, then this will need to be separated out into
	//         two loops
	while (i < max) {
		switch (data[i]) {
			case ENTRY_TYPE_LAPIC: {
				uint32_t uid = *(uint8_t *)(data + i + 2);
				uint32_t id = *(uint8_t *)(data + i + 3);
				uint32_t flags = *(uint32_t *)(data + i + 4);

				ARC_DEBUG(INFO, "LAPIC found (UID: %d, ID: %d, Flags: 0x%x)\n", uid, id, flags);

				init_smp(id, uid, flags, 0xFF);

				break;
			}

			case ENTRY_TYPE_IOAPIC: {
				// Disable 8259
				outb(0x21, 0xFF);
				outb(0xA1, 0xFF);

				uint32_t address = *(uint32_t *)(data + i + 4);
				uint32_t gsi = *(uint32_t *)(data + i + 8);
				uint32_t id = *(uint8_t *)(data + i + 2);

				ARC_DEBUG(INFO, "IOAPIC found (GSI: %d, Address: 0x%"PRIx32", ID: %d)\n", gsi, address, id);

				int mre = init_ioapic(address);

				if (mre <= 0) {
					ARC_DEBUG(ERR, "\tFailed to initialize IOAPIC\n");
				}

				struct ioapic_element *next = (struct ioapic_element *)alloc(sizeof(struct ioapic_element));

				if (next == NULL) {
					ARC_DEBUG(ERR, "\tFailed to allocate IOAPIC descriptor\n");
					break;
				}

				next->gsi = gsi;
				next->mre = mre;
				next->ioapic = (struct ioapic_register *)((uintptr_t)address);
				next->id = id;

				next->next = ioapic_list;
				ioapic_list = next;

				break;
			}

			case ENTRY_TYPE_INT_OVERRIDE_SRC: {
				uint8_t irq = *(uint8_t *)(data + i + 3);
				uint32_t gsi = *(uint32_t *)(data + i + 4);
				uint16_t flags = *(uint16_t *)(data + i + 8);
				uint8_t polarity = (flags       & 0b11) == 0b11 ? 0x1 : 0x0; // 01: Active High; 11: Active Low
				uint8_t trigger = ((flags >> 2) & 0b11) == 0b11 ? 0x1 : 0x0; // 01: Edge; 11: Level

				ARC_DEBUG(INFO, "Interrupt Source Override found (IRQ: %d, GSI: %d, %s, %s)\n", irq, gsi, polarity ? "Active Low" : "Active High", trigger ? "Level Sensitive" : "Edge Sensitive");

				if (irq != 0) {
					// The LAPIC timer will be used
	//				apic_map_gsi_irq(gsi, irq, bsp, (trigger | (polarity << 1)));
				}

				break;
			}

			case ENTRY_TYPE_LAPIC_NMI: {
				ARC_DEBUG(WARN, "LAPIC NMI found\n");

				break;
			}

			default: {
				ARC_DEBUG(WARN, "Unhandled MADT entry of type %d\n", data[i]);
				break;
			}		
		}

		i += data[i + 1];
	}

	return 0;
}
