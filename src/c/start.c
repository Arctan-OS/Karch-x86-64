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
#include <arch/start.h>
#include <arch/acpi/acpi.h>
#include <arch/pci/pci.h>

#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/apic/apic.h>

int init_arch() {
        if (init_acpi() != 0) {
		return -1;
	}

        if (init_apic() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
		ARC_HANG;
	}

	__asm__("sti");

	if (init_pci() != 0) {
		return -2;
	}

	return 0;
}
