/**
 * @file lapic.c
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
#include <cpuid.h>

#include "arch/pager.h"
#include "arch/x86-64/apic/local.h"
#include "arch/x86-64/ctrl_regs.h"
#include "global.h"
#include "util.h"

#define ADDRESS_MASK 0x0000FFFFFFFFFFFF
#define GET_LAPIC_MSR _x86_RDMSR(0x1B)
#define SET_LAPIC_MSR(_lapic_msr) _x86_WRMSR(0x1B, _lapic_msr)
#define GET_LAPIC_REG(_lapic_msr) (ARC_LAPICReg *)(((_lapic_msr >> 12) & ADDRESS_MASK) << 12)

typedef struct ARC_LAPICReg {
        uint32_t resv0 __attribute__((aligned(16)));
        uint32_t resv1 __attribute__((aligned(16)));
        uint32_t lapic_id __attribute__((aligned(16)));
        uint32_t lapic_ver __attribute__((aligned(16)));
        uint32_t resv2 __attribute__((aligned(16)));
        uint32_t resv3 __attribute__((aligned(16)));
        uint32_t resv4 __attribute__((aligned(16)));
        uint32_t resv5 __attribute__((aligned(16)));
        uint32_t tpr __attribute__((aligned(16)));
        uint32_t apr __attribute__((aligned(16)));
        uint32_t ppr __attribute__((aligned(16)));
        uint32_t eoi_reg __attribute__((aligned(16)));
        uint32_t rrd __attribute__((aligned(16)));
        uint32_t logical_dest_reg __attribute__((aligned(16)));
        uint32_t dest_form_reg __attribute__((aligned(16)));
        uint32_t spurious_int_vector __attribute__((aligned(16)));
        uint32_t isr0 __attribute__((aligned(16)));
        uint32_t isr1 __attribute__((aligned(16)));
        uint32_t isr2 __attribute__((aligned(16)));
        uint32_t isr3 __attribute__((aligned(16)));
        uint32_t isr4 __attribute__((aligned(16)));
        uint32_t isr5 __attribute__((aligned(16)));
        uint32_t isr6 __attribute__((aligned(16)));
        uint32_t isr7 __attribute__((aligned(16)));
        uint32_t tmr0 __attribute__((aligned(16)));
        uint32_t tmr1 __attribute__((aligned(16)));
        uint32_t tmr2 __attribute__((aligned(16)));
        uint32_t tmr3 __attribute__((aligned(16)));
        uint32_t tmr4 __attribute__((aligned(16)));
        uint32_t tmr5 __attribute__((aligned(16)));
        uint32_t tmr6 __attribute__((aligned(16)));
        uint32_t tmr7 __attribute__((aligned(16)));
        uint32_t irr0 __attribute__((aligned(16)));
        uint32_t irr1 __attribute__((aligned(16)));
        uint32_t irr2 __attribute__((aligned(16)));
        uint32_t irr3 __attribute__((aligned(16)));
        uint32_t irr4 __attribute__((aligned(16)));
        uint32_t irr5 __attribute__((aligned(16)));
        uint32_t irr6 __attribute__((aligned(16)));
        uint32_t irr7 __attribute__((aligned(16)));
        uint32_t err_stat_reg __attribute__((aligned(16)));
        uint32_t resv6 __attribute__((aligned(16)));
        uint32_t resv7 __attribute__((aligned(16)));
        uint32_t resv8 __attribute__((aligned(16)));
        uint32_t resv9 __attribute__((aligned(16)));
        uint32_t resv10 __attribute__((aligned(16)));
	uint32_t resv11 __attribute__((aligned(16)));
        uint32_t lvt_cmci_reg __attribute__((aligned(16)));
        uint32_t icr0 __attribute__((aligned(16))); // Lower
        uint32_t icr1 __attribute__((aligned(16))); // Upper
        uint32_t lvt_timer_reg __attribute__((aligned(16)));
        uint32_t lvt_thermal_reg __attribute__((aligned(16)));
        uint32_t lvt_preformance_reg __attribute__((aligned(16)));
        uint32_t lvt_lint0_reg __attribute__((aligned(16)));
        uint32_t lvt_lint1_reg __attribute__((aligned(16)));
        uint32_t lvt_err_reg __attribute__((aligned(16)));
        uint32_t init_count_reg __attribute__((aligned(16)));
        uint32_t cur_count_reg __attribute__((aligned(16)));
        uint32_t resv12 __attribute__((aligned(16)));
        uint32_t resv13 __attribute__((aligned(16)));
        uint32_t resv14 __attribute__((aligned(16)));
        uint32_t resv15 __attribute__((aligned(16)));
        uint32_t div_conf_reg __attribute__((aligned(16)));
        uint32_t resv16 __attribute__((aligned(16)));
}__attribute__((packed)) ARC_LAPICReg;
STATIC_ASSERT(sizeof(ARC_LAPICReg) == 0x400, "LAPIC reg wrong size, something may be missing");

void lapic_eoi() {
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);
	reg->eoi_reg = 0x0;
}

void lapic_ipi(uint8_t vector, uint8_t destination, uint32_t flags) {
	// NOTE: See Intel SDM Vol. 3 11.6.1 for information on the
	//       values of the above bit fields

	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	reg->icr1 = destination << 24;
	reg->icr0 = vector | flags;
}

int lapic_ipi_poll() {
	// Returns the delivery status
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	return (reg->icr0 >> 12) & 1;
}

int lapic_get_id() {
        register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

        __cpuid(0x01, eax, ebx, ecx, edx);

        if (((edx >> 9) & 1) == 0) {
                ARC_DEBUG(INFO, "No APIC on chip\n");
                return -1;
        }

	return (ebx >> 24) & 0xFF;
}

int lapic_calibrate_timer() {
	ARC_DEBUG(WARN, "Definitely calibrating LAPIC timer using the HPET\n");
	return 0;
}

void lapic_setup_timer(uint8_t vector, uint8_t mode) {
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	reg->lvt_timer_reg = vector | ((mode & 0b11) << 17);
}

void lapic_timer_mask(uint8_t mask) {
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	if (mask) {
		reg->lvt_timer_reg |= 1 << 16;
	} else {
		reg->lvt_timer_reg &= ~(1 << 16);
	}
}

void lapic_refresh_timer(uint32_t count) {
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	reg->init_count_reg = count;
}

void lapic_divide_timer(uint8_t division) {
	ARC_LAPICReg *reg = GET_LAPIC_REG(GET_LAPIC_MSR);

	reg->div_conf_reg = (division & 0b11) | ((division >> 2) & 1) << 3;
}

int init_lapic() {
	int id = lapic_get_id();

	if (id == -1) {
		return -1;
	}

        ARC_DEBUG(INFO, "Initializing LAPIC\n");

	uint64_t lapic_msr = GET_LAPIC_MSR;
        ARC_LAPICReg *reg = GET_LAPIC_REG(lapic_msr);

        if (((lapic_msr >> 8) & 1) == 1) {
                ARC_DEBUG(INFO, "BSP LAPIC\n");
        }

        lapic_msr |= (1 << 11);
        SET_LAPIC_MSR(lapic_msr);

	if (pager_map(NULL, (uint64_t)reg, (uint64_t)reg, PAGE_SIZE, 1 << ARC_PAGER_RW | ARC_PAGER_PAT_UC) != 0) {
		ARC_DEBUG(ERR, "Failed to map LAPIC register\n");
	}

        ARC_DEBUG(INFO, "LAPIC register at %p\n", reg);
        // NOTE: Ignore bits 31:27 of reg->lapic_id on P6 and Pentium processors
        uint8_t ver = reg->lapic_ver & 0xFF;

        ARC_DEBUG(INFO, "LAPIC ID: 0x%X (0x%X) (%s)\n", reg->lapic_id >> 28, id, ((reg->spurious_int_vector >> 8) & 1) ? "disabled, enabling" : "enabled");
	// Enable LAPIC
	reg->spurious_int_vector |= 1 << 8;
        ARC_DEBUG(INFO, "\tVersion: %d (%s)\n", ver, ver < 0xA ? "82489DX discrete APIC" : "Integrated APIC");
        ARC_DEBUG(INFO, "\tMax LVT: %d+1\n", ((reg->lapic_ver >> 16) & 0xFF));
        ARC_DEBUG(INFO, "\tEOI-broadcast supression: %s\n", (reg->lapic_ver >> 24) & 1 ? "yes" : "no");

        ARC_DEBUG(INFO, "Successfully initialized LAPIC\n");

        return id;
}
