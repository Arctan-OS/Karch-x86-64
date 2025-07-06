/**
 * @file info.c
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
#include <global.h>
#include <arch/info.h>
#include <cpuid.h>

#define DECLARE_REGISTERS32 \
        register uint32_t eax = 0; \
        register uint32_t ebx = 0; \
        register uint32_t ecx = 0; \
        register uint32_t edx = 0; \

uint32_t arch_physical_address_width() {
        DECLARE_REGISTERS32;

        __cpuid(0x80000008, eax, ebx, ecx, edx);

        return MASKED_READ(eax, 0, 0xFF);
}

uint32_t arch_virtual_address_width() {
        DECLARE_REGISTERS32;

        __cpuid(0x80000008, eax, ebx, ecx, edx);

        return MASKED_READ(eax, 8, 0xFF);
}
