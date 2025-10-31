/**
 * @file syscall.c
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
#include "arch/x86-64/smp.h"
#include "config.h"
#include <arch/io/port.h>
#include <mm/pmm.h>
#include <global.h>
#include <arch/syscall.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mp/scheduler.h>
#include <arch/smp.h>
#include <stdint.h>

uintptr_t USERSPACE(text) syscall_get_kpages() {
	return ARC_HHDM_TO_PHYS(Arc_CurProcessorDescriptor->descriptor.process->page_tables.kernel);
}

extern int _syscall();
int init_syscall() {
	uint64_t ia32_fmask = 0;
	_x86_WRMSR(0xC0000084, ia32_fmask);

	uint64_t ia32_lstar = _x86_RDMSR(0xC0000082);
	ia32_lstar = (uintptr_t)_syscall;
	_x86_WRMSR(0xC0000082, ia32_lstar);

	uint64_t ia32_star = _x86_RDMSR(0xC0000081);
	uint64_t syscall_ss_cs = 0x08;
	uint32_t syscall_eip = 0x0;
	uint64_t sysret_ss_cs = 0x10; // CS: This value + 0x10, SS: This value + 0x8
	ia32_star |= sysret_ss_cs << 48 | syscall_ss_cs << 32 | syscall_eip;
	_x86_WRMSR(0xC0000081, ia32_star);

	uint64_t ia32_efer = _x86_RDMSR(0xC0000080);
	ia32_efer |= 1;
	_x86_WRMSR(0xC0000080, ia32_efer);

	ARC_DEBUG(INFO, "Installed syscalls\n");
	ARC_DEBUG(INFO, "\tEFER: 0x%lX\n", ia32_efer);
	ARC_DEBUG(INFO, "\tSTAR: 0x%lX\n", ia32_star);
	ARC_DEBUG(INFO, "\tLSTAR: 0x%lX\n", ia32_lstar);

	return 0;
}
