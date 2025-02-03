/**
 * @file ioapic.c
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
#include <arch/x86-64/apic/ioapic.h>
#include <global.h>
#include <mm/allocator.h>
#include <arch/pager.h>
#include <mm/vmm.h>
#include <stdint.h>

uint32_t ioapic_read_register(struct ioapic_register *ioapic, uint32_t reg) {
	ioapic->ioregsel = reg;
	return ioapic->iowin;
}

int ioapic_write_register(struct ioapic_register *ioapic, uint32_t reg, uint32_t value) {
	ioapic->ioregsel = reg;
	ioapic->iowin = value;

	return 0;
}

int ioapic_write_redir_tbl(struct ioapic_register *ioapic, int table_idx, struct ioapic_redir_tbl *table) {
	int low_dword_i = (table_idx * 2) + 0x10;
	int high_dword_i = low_dword_i + 1;

	uint64_t value = *(uint64_t *)table;

	ioapic_write_register(ioapic, low_dword_i, value & UINT32_MAX);
	ioapic_write_register(ioapic, high_dword_i, (value >> 32) & UINT32_MAX);

	return 0;
}

uint64_t ioapic_read_redir_tbl(struct ioapic_register *ioapic, int table_idx) {
	int low_dword_i = (table_idx * 2) + 0x10;
	int high_dword_i = low_dword_i + 1;

	return (uint64_t)(ioapic_read_register(ioapic, low_dword_i) | ((uint64_t)ioapic_read_register(ioapic, high_dword_i) << 32));
}

uint32_t init_ioapic(uint32_t address) {
	struct ioapic_register *ioapic = (struct ioapic_register *)((uintptr_t)address);

	int map_res = pager_map(NULL, (uintptr_t)ioapic, (uintptr_t)ioapic, PAGE_SIZE, 1 << ARC_PAGER_4K | ARC_PAGER_PAT_UC);

	if (map_res != 0 && map_res != -5) {
		ARC_DEBUG(ERR, "Mapping failed\n");
		return 0;
	}

	uint32_t ver = ioapic_read_register(ioapic, 0x01);

	return ((ver >> 16) & 0xFF);
}
