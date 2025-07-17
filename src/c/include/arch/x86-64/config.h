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

#define PAGE_SIZE_LOWEST_EXPONENT 12
#define PAGE_SIZE (size_t)(1 << PAGE_SIZE_LOWEST_EXPONENT)

#ifndef PMM_BIAS_ARRAY
// SMALLEST_PAGE_SIZE_EXPONENT should not be included
// in the list below as those are handled in a separate
// freelist with less overhead
// A list of exponents to dictate block size and the order
// in which they should be tried (priority). The priority
// descends from left to right
#define PMM_BIAS_ARRAY 30,21
#endif

#ifndef PMM_BIAS_LOW
// These are the minimum nuber of each block size
// listed above that must fit into a region
#define PMM_BIAS_LOW 4,12
#endif

#ifndef PMM_BIAS_RATIO
// These are the maximum nuber of each block size
// listed above that are to be initialized
//
// Values == 0 will be processed last
#define PMM_BIAS_RATIO 2,1
#define PMM_BIAS_DENOMINATOR 3

// PMM_BIAS_RATIO[i] / PMM_BIAS_DENOMINATOR

#endif

#ifndef PMM_BUDDY_LOWEST_EXPONENT
#define PMM_BUDDY_LOWEST_EXPONENT PAGE_SIZE_LOWEST_EXPONENT
#endif

#define PMM_BUDDY_LOWEST_SIZE (1 << PMM_BUDDY_LOWEST_EXPONENT)

#endif
