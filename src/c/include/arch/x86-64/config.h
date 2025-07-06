/**
 * @file config.h
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
#ifndef ARC_ARCH_X86_64_CONFIG_H
#define ARC_ARCH_X86_64_CONFIG_H

#include <stddef.h>

#define SMALLEST_PAGE_SIZE_EXPONENT 12
#define PAGE_SIZE (size_t)(1 << SMALLEST_PAGE_SIZE_EXPONENT)

#ifndef PMM_BIAS_ARRAY
// SMALLEST_PAGE_SIZE_EXPONENT should not be included
// in the list below as those are handled in a separate
// freelist with less overhead

// A list of exponents to dictate block size and the order
// in which they should be tried (priority). The priority
// descends from left to right
#define PMM_BIAS_ARRAY 30,21

// These are the coefficients to the block size. Dictating
// the minimum number of blocks that must fit into a region
// for that region to be converted to a freelist of those
// blocks
#define PMM_BIAS_COEFF 4,12
// The number of elements in the array and the number of
// coefficients
#define PMM_BIAS_COUNT 2
#endif

#endif
