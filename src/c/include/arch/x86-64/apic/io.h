/**
 * @file ioapic.h
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
#ifndef ARC_ARCH_X86_64_APIC_IO_H
#define ARC_ARCH_X86_64_APIC_IO_H

#include <stdint.h>

typedef struct ARC_IOAPICReg {
        uint32_t ioregsel __attribute__((aligned(16)));
        uint32_t iowin __attribute__((aligned(16)));
}__attribute__((packed)) ARC_IOAPICReg;

typedef struct ARC_IOAPICRedirTable {
        uint8_t int_vec;
        uint8_t del_mod : 3;
        uint8_t dest_mod : 1;
        uint8_t del_stat : 1;
        uint8_t int_pol : 1;
        uint8_t irr : 1;
        uint8_t trigger : 1;
        uint8_t mask : 1;
        uint64_t resv0 : 39;
        uint8_t destination;
}__attribute__((packed)) ARC_IOAPICRedirTable;

uint32_t ioapic_read_register(ARC_IOAPICReg *ioapic, uint32_t reg);
int ioapic_write_register(ARC_IOAPICReg *ioapic, uint32_t reg, uint32_t value);
int ioapic_write_redir_tbl(ARC_IOAPICReg *ioapic, int table_idx, ARC_IOAPICRedirTable *table);
uint64_t ioapic_read_redir_tbl(ARC_IOAPICReg *ioapic, int table_idx);
uint32_t init_ioapic(uint32_t address);

#endif
