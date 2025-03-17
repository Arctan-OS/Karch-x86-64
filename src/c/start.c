/**
 * @file start.c
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
#include "loaders/elf.h"
#include <arch/start.h>
#include <arch/acpi/acpi.h>
#include <arch/pci/pci.h>
#include <arch/process.h>
#include <arch/smp.h>

#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/apic/apic.h>
#include <arch/x86-64/sse.h>

struct ARC_Process *Arc_ProcessorHold = NULL;

int init_arch() {
        if (init_acpi() != 0) {
		return -1;
	}

        if (init_apic() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
		ARC_HANG;
	}

	if (init_pci() != 0) {
		return -2;
	}

	Arc_ProcessorHold = process_create(0, NULL);

	if (Arc_ProcessorHold == NULL) {
		ARC_DEBUG(ERR, "Failed to create hold process\n");
		__asm__("cli");
		ARC_HANG;
	}

	struct ARC_Thread *hold = thread_create(Arc_ProcessorHold->page_tables, (void *)smp_hold, 0);

	if (hold == NULL) {
		ARC_DEBUG(ERR, "Failed to create hold thread\n");
		process_delete(Arc_ProcessorHold);
		__asm__("cli");
		ARC_HANG;
	}

	process_associate_thread(Arc_ProcessorHold, hold);

	init_sse();

	return 0;
}
