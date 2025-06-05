/**
 * @file floats.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Kernel - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan
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
 * Called by CPUID to enable support for SSE.
*/

#include "arch/x86-64/context.h"
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <arch/floats.h>
#include <cpuid.h>
#include <mm/allocator.h>
#include <lib/util.h>

// NOTE: It may be valuable to create a mechanism that utilizes xsave and xrstor
//       instead of fxsave and fxrstor. This may become its own thing as xsave/rstor
//       allow for the saving and restoring of many other bits of data other than just
//       floating point data. For now though, fxsave/rstor are used.

static int init_sse(struct ARC_Context *context) {
	register uint32_t eax;
	register uint32_t ebx;
	register uint32_t ecx;
	register uint32_t edx;

	__cpuid(0x01, eax, ebx, ecx, edx);

	if (((edx >> 24) & 1) == 0) {
		ARC_DEBUG(ERR, "No fxsave/rstor instructions\n");
		return -1;
	}

	if ((((edx >> 25) & 1) == 0 && ((edx >> 26) & 1) == 0
	    && (ecx & 1) == 0 && ((ecx >> 9) & 1) == 0)) {
		ARC_DEBUG(ERR, "No support for SSE\n");
		return -2;
	}

	uint64_t cr0 = _x86_getCR0();
	cr0 &= ~(1 << 2); // Disable x87 FPU emulation
	cr0 |= (1 << 1);
	_x86_setCR0(cr0);

	uint64_t cr4 = _x86_getCR4();
	cr4 |= (1 << 9); // OSFXSR
	cr4 |= (1 << 10); // OSXMMEXCPT
	_x86_setCR4(cr4);

	void *fxsave_space = alloc(512);
	
	if (fxsave_space == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate fxsave space\n");
		return -3;
	}

	memset(fxsave_space, 0, 512);
	
	context->fxsave_space = fxsave_space;
	context->cr0 = cr0;
	context->cr4 = cr4;

	return 0;
}

int init_floats(struct ARC_Context *context, int flags) {
	(void)flags;

	return init_sse(context);
}