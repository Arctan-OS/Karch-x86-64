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
#include <arctan.h>

#define PAGE_SIZE_LOWEST_EXPONENT 12
#define PAGE_SIZE (size_t)(1 << PAGE_SIZE_LOWEST_EXPONENT)

#ifndef ARC_PMM_LOW_MEM_LIM
// The first address from zero that is in high memory.
// Where a is an address:
// 0 <= a < ARC_PMM_LOW_MEM_LIM -> Low Memory Address
// ARC_PMM_LOW_MEM_LIM < a -> High Memory Address
#define ARC_PMM_LOW_MEM_LIM 0x10000
#endif

#ifndef ARC_PMM_BIASES_DEFINED
#define ARC_PMM_BIASES_DEFINED
static const struct ARC_PMMBiasConfigElement pmm_biases_high[] = {
        {
                .exp = 30,
                .min_blocks = 4,
                .min_buddy_exp = PAGE_SIZE_LOWEST_EXPONENT,
                .ratio.numerator = 2,
                .ratio.denominator = 3,
        },
        {
                .exp = 21,
                .min_blocks = 12,
                .min_buddy_exp = PAGE_SIZE_LOWEST_EXPONENT,
                .ratio.numerator = 2,
                .ratio.denominator = 3,
        },
};

static const struct ARC_PMMBiasConfigElement pmm_biases_low[] = {
        {
                .exp = 13,
                .min_blocks = 1,
                .min_buddy_exp = PAGE_SIZE_LOWEST_EXPONENT,
                .ratio.numerator = 2,
                .ratio.denominator = 3,
        },
};
#endif

#endif
