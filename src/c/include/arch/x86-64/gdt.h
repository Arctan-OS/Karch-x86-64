/**
 * @file gdt.h
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
 * Change out the GDT to better suit 64-bit mode, remove no longer needed 32-bit
 * segments.
*/
#ifndef ARC_ARCH_X86_64_GDT_H
#define ARC_ARCH_X86_64_GDT_H

#include <stdint.h>

/**
 * Initialize the GDT
 *
 *
 * Initialize and load the GDT with basic gate descriptors for
 * kernel code, kernel data, user code, user data.
 * */

uintptr_t init_gdt(int processor);

#endif
