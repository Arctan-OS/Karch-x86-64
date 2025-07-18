/**
 * @file start.c
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
*/
#include <arch/start.h>
#include <arch/smp.h>
#include <arch/x86-64/apic/apic.h>
#include <arch/x86-64/idt.h>
#include <arch/pager.h>
#include <global.h>

int init_arch_early() {
	idt_install_exceptions(0x18, 0);
	init_idt();

	return 0;
}

int init_arch() {
        if (init_apic() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
		ARC_HANG;
	}

	return 0;
}
