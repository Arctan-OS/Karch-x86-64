/**
 * @file sse.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
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

#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <arch/x86-64/sse.h>
#include <cpuid.h>
#include <mm/pmm.h>

/**
 * External assembly function to enable SSE.
 * */
extern void _osxsave_support(uintptr_t fxsave_addr);

int init_sse() {
	register uint32_t eax;
	register uint32_t ebx;
	register uint32_t ecx;
	register uint32_t edx;

	__cpuid(0x01, eax, ebx, ecx, edx);

        // Check for SSE2
	if (((edx >> 25) & 1) == 0 && ((edx >> 26) & 1) == 0
	    && (ecx & 1) == 0 && ((ecx >> 9) & 1) == 0) {
		ARC_DEBUG(INFO, "No SSE/SSE2/SSE3/SSSE3 extensions\n");
		return -1;
	}

	_x86_getCR0();
	_x86_CR0 &= ~(1 << 2); // Disable x87 FPU emulation
	_x86_CR0 |= (1 << 1); //
	_x86_setCR0();

	_x86_getCR4();
	_x86_CR4 |= (1 << 9); // OSFXSR
	_x86_CR4 |= (1 << 10); // OSXMMEXCPT
	_x86_setCR4();

	if (((ecx >> 27) & 1) == 1) {
		_x86_CR4 |= (1 << 18); // OSXSAVE support
		_x86_setCR4();
		void *fxsave_space = pmm_alloc();

		if (fxsave_space == NULL) {
			ARC_DEBUG(ERR, "Failed to a llocated FXSAVE space\n");
			// Not sure what to do here, SSE is needed from here on
			ARC_HANG;
		}

		_osxsave_support((uintptr_t)fxsave_space - 0x10);
	}

	// Set MXCSR Register
	ARC_DEBUG(INFO, "Enabled SSE support\n");

	return 0;
}
