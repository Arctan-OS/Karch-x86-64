/**
 * @file cmos.c
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
#include <arch/x86-64/cmos.h>
#include <arch/io/port.h>

uint8_t cmos_read(uint8_t reg) {
	// Disable NMI
	outb(0x70, (1 << 7) | reg);
	return inb(0x71);
}

int cmos_write(uint8_t reg, uint8_t value) {
	// Disable NMI
	outb(0x70, (1 << 7) | reg);
	outb(0x71, value);
	return 0;
}
