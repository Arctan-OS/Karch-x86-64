/**
 * @file floats.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Karch-x86-64 - x86-64 Implementation of K/arch Abstractions
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan
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
 * Called by CPUID to enable support for SSE.
*/

#include "arch/x86-64/context.h"
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>

#include <mm/allocator.h>
#include <lib/util.h>

// NOTE: It may be valuable to create a mechanism that utilizes xsave and xrstor
//       instead of fxsave and fxrstor. This may become its own thing as xsave/rstor
//       allow for the saving and restoring of many other bits of data other than just
//       floating point data. For now though, fxsave/rstor are used.

int init_sse(ARC_Context * context) {
        // TODO: Xsave needs more bytes
	void *fxsave_space = alloc(512);

        if (fxsave_space == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate fxsave space\n");
		return -3;
        }

        memset(fxsave_space, 0, 512);

        context->xsave_space = fxsave_space;

	context->frame.gpr.cr0 &= ~(1 << 2); // Disable x87 FPU emulation
	context->frame.gpr.cr0 |= (1 << 1);

	context->frame.gpr.cr4 |= (1 << 10); // OSXMMEXCPT

	ARC_DEBUG(INFO, "Initalized SSE\n");

	return 0;
}
