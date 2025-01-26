/**
 * @file pager.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan.
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
#ifndef ARC_ARCH_X86_64_PAGER_H
#define ARC_ARCH_X86_64_PAGER_H

#include <stdint.h>
#include <stddef.h>
#include <arch/pager.h>

extern uintptr_t Arc_KernelPageTables;

/**
 * Map the given physical to the given virtual.
 *
 * Associates the range [virtual, virtual + size) with
 * the range [physical, physical + size) and sets all page
 * entries with the given attributes.
 *
 * NOTE: This function must only be used in conjunction with
 *       pager_unmap!
 *
 * @param uintptr_t virtual - The base virtual address to map.
 * @param uintptr_t physical - The base physical address to map.
 * @param size_t size - The length of the physical and virtual ranges in bytes.
 * @param uint32_t attributes - Attributes for page entries (see above ARC_PAGER_* definitions for bit offsets)
 * @return zero on success.
 * */
int pager_map(void *cr3, uintptr_t virtual, uintptr_t physical, size_t size, uint32_t attributes) ;

/**
 * Unmap the given virtual range.
 *
 * NOTE: This function must only be used in conjunction with
 *       pager_map
 *
 * @param uintptr_t virtual - The base virtual address to unmap.
 * @param size_t size - The length of the virtual range in bytes.
 * @return zero on success.
 * */
int pager_unmap(void *cr3, uintptr_t virtual, size_t size);

/**
 *
 *
 * @return zero on success.
 * */
int pager_fly_map(void *cr3, uintptr_t virtual, size_t size, uint32_t attributes);

/**
 *
 * @ return zero on success.
 * */
int pager_fly_unmap(void *cr3, uintptr_t virtual, size_t size);

/**
 * Set paging attributes in the given range.
 *
 * Set the various paging attributes for page entries.
 * See the above definitions following ARC_PAGER_* for the
 * offsets for attributes parameter.
 *
 * @param uintptr_t virtual - The base virtual address.
 * @param size_t size - The size of the virtual range in bytes.
 * @param uint32_t attributes - The attributes to set.
 * @return zero on success.
 * */
int pager_set_attr(void *cr3, uintptr_t virtual, size_t size, uint32_t attributes);

int pager_clone(void *cr3, uintptr_t virt_src, uintptr_t virt_dest, size_t size);

/**
 * Initialize the x86-64 pager.
 * */
int init_pager();

#endif
