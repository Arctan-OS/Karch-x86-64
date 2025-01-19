/**
 * @file pager.c
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
#include <arch/x86-64/pager.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/pmm.h>
#include <global.h>
#include <lib/util.h>

#define ADDRESS_MASK 0x000FFFFFFFFFF000
// Flags for Arc_BootMeta->paging_features
#define FLAGS_NO_EXEC (1 << 0)
// NOTE: Since PML4 is used 2MiB pages
//       ought to be supported (I have not
//       found a way to test for them)
#define FLAGS_1GIB (1 << 1)

#define ONE_GIB 0x40000000
#define TWO_MIB 0x200000

struct pager_traverse_info {
	void *cr3;
	uintptr_t virtual;
	uintptr_t physical;
	size_t size;
	uint32_t attributes;
};

// Value current loaded in CR3
static uint64_t *pml4 = NULL;

uint64_t get_entry_bits(int level, uint32_t attributes) {
	// Level 0: Page
	// Level 1: PT
	// Level 2: PD
	// Level 3: PDP
	// Level 4: PML4

	uint64_t bits = 0;

	bits |= ((attributes >> (ARC_PAGER_PAT)) & 1) << 2; // PWT
	bits |= ((attributes >> (ARC_PAGER_PAT + 1)) & 1) << 3; // PCD

	switch (level) {
		case 1: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 7;

			break;
		}

		case 2: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 2MiB pages unless 4K specified
			if (((attributes >> ARC_PAGER_RESV1) & 1) == 1) {
				bits |= (!((attributes >> (ARC_PAGER_4K)) & 1)) << 7;
			}

			break;
		}

		case 3: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 1GiB pages unless 4K specified
 			if (((attributes >> ARC_PAGER_RESV0) & 1) == 1) {
				bits |= (!((attributes >> (ARC_PAGER_4K)) & 1)) << 7;
			}

			break;
		}

		// On 4, 5, bit 7 is reserved
	}

	bits |= ((attributes >> ARC_PAGER_US) & 1) << 2;
	bits |= ((attributes >> ARC_PAGER_RW) & 1) << 1;
	bits |= 1; // Present

	if ((Arc_BootMeta->paging_features & FLAGS_NO_EXEC) == 1) {
		bits |= (uint64_t)((attributes >> ARC_PAGER_NX) & 1) << 63;
	}

	return bits;
}

/**
 * Get the next page table.
 *
 * @param uint64_t *parent - The parent table.
 * @param int level - The level of the parent table (4 = PML4).
 * @param uintptr_t virtual - The base virtual address.
 * @param uint32_t attributes - Specified attributes.
 * @parma int *index - The 64-bit entry in the parent corresponding to the returned table.
 * @return a non-NULL pointer to the next page table
 * */
static int get_page_table(uint64_t *parent, int level, uintptr_t virtual, uint32_t attributes) {
	int shift = ((level - 1) * 9) + 12;
	int index = (virtual >> shift) & 0x1FF;

	uint64_t entry = parent[index];

	uint64_t *address = (uint64_t *)ARC_PHYS_TO_HHDM(entry & 0x0000FFFFFFFFF000);

	bool present = entry & 1;
	bool only_4k = (attributes >> ARC_PAGER_4K) & 1;
	bool can_gib = (attributes >> ARC_PAGER_RESV0) & 1;
	bool can_mib = (attributes >> ARC_PAGER_RESV1) & 1;
	bool no_create = (attributes >> ARC_PAGER_RESV2) & 1;

	if (!present && (only_4k || (level == 4 && !can_gib) || (level == 3 && !can_mib) || level == 2) && !no_create) {
		// Only make a new table if:
		//     The current entry is not present AND:
		//         - Mapping is only 4K, or
		//         - Can't make a GiB page, or
		//         - Can't make a MiB page, or
		//         - Parent is a level 2 page table
		//     AND creation of page tables is allowed

		address = (uint64_t *)pmm_alloc();

		if (address == NULL) {
			return -1;
		}

		memset(address, 0, PAGE_SIZE);

		parent[index] = (uintptr_t)ARC_HHDM_TO_PHYS(address) | get_entry_bits(level, attributes);
	}

	return index;
}

/**
 * Standard function to traverse x86-64 page tables
 *
 * @param struct pager_traverse_info *info - Information to use for traversing and to pass to the callback.
 * @param int *(callback)(...) - The callback function.
 * @return zero on success.
 * */
static int pager_traverse(struct pager_traverse_info *info, int (*callback)(struct pager_traverse_info *info, uint64_t *table, int index, int level)) {
	if (info == NULL || info->cr3 == NULL) {
		return -1;
	}

	if (info->size == 0) {
		return 0;
	}

	uintptr_t target = info->virtual + info->size;

	while (info->virtual < target) {
		uint64_t *table = info->cr3;
		int index = get_page_table(table, 4, info->virtual, info->attributes);

		table = (uint64_t *)ARC_PHYS_TO_HHDM(table[index] & ADDRESS_MASK);
		index = get_page_table(table, 3, info->virtual, info->attributes);
		bool create_large = ((info->attributes >> ARC_PAGER_RESV0) & 1) & ~((info->attributes >> ARC_PAGER_4K) & 1);

		if (info->size > ONE_GIB && create_large) {
			// Map 1 GiB page
			if (callback(info, table, index, 3) != 0) {
				return -3;
			}

			info->virtual += ONE_GIB;
			info->physical += ONE_GIB;
			info->size -= ONE_GIB;

			continue;
		}

		table = (uint64_t *)ARC_PHYS_TO_HHDM(table[index] & ADDRESS_MASK);
		index = get_page_table(table, 2, info->virtual, info->attributes);
		create_large = ((info->attributes >> ARC_PAGER_RESV1) & 1) & ~((info->attributes >> ARC_PAGER_4K) & 1);

		if (info->size > TWO_MIB && create_large) {
			// Map 2 MiB page
			if (callback(info, table, index, 2) != 0) {
				return -2;
			}

			info->virtual += TWO_MIB;
			info->physical += TWO_MIB;
			info->size -= TWO_MIB;

			continue;
		}

		table = (uint64_t *)ARC_PHYS_TO_HHDM(table[index] & ADDRESS_MASK);
		index = get_page_table(table, 1, info->virtual, info->attributes);

		// Map 4K page
		if (callback(info, table, index, 1) != 0) {
			return -1;
		}

		info->virtual += PAGE_SIZE;
		info->physical += PAGE_SIZE;
		info->size -= PAGE_SIZE;
	}

	return 0;
}

static int pager_map_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}

	table[index] = info->physical | get_entry_bits(level, info->attributes);

	if (info->cr3 == pml4) {
		__asm__("invlpg [%0]" : : "r"(info->virtual) : );
	}

	return 0;
}

int pager_map(void *cr3, uintptr_t virtual, uintptr_t physical, size_t size, uint32_t attributes) {
	struct pager_traverse_info info = { .virtual = virtual, .physical = physical,
	                                    .size = size, .attributes = attributes,
					    .cr3 = cr3 == NULL ? pml4 : cr3 };

	if (pager_traverse(&info, pager_map_callback) != 0) {
		ARC_DEBUG(ERR, "Failed to map P0x%"PRIx64":V0x%"PRIx64" (0x%"PRIx64" B, 0x%x)\n", physical, virtual, size, attributes);
		return -1;
	}

	return 0;
}

static int pager_unmap_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}

	table[index] = 0;

	if (info->cr3 == pml4) {
		__asm__("invlpg [%0]" : : "r"(info->virtual) : );
	}

	return 0;
}

int pager_unmap(void *cr3, uintptr_t virtual, size_t size) {
	struct pager_traverse_info info = { .virtual = virtual, .size = size, .cr3 = cr3 == NULL ? pml4 : cr3};

	if (pager_traverse(&info, pager_unmap_callback) != 0) {
		ARC_DEBUG(ERR, "Failed to map V0x%"PRIx64" (0x%"PRIx64" B)\n", virtual, size);
		return -1;
	}

	return 0;
}

static int pager_fly_map_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}

	void *page = pmm_alloc();

	if (page == NULL) {
		return -2;
	}

	table[index] = ARC_HHDM_TO_PHYS(page) | get_entry_bits(level, info->attributes);

	if (info->cr3 == pml4) {
		__asm__("invlpg [%0]" : : "r"(info->virtual) : );
	}

	return 0;
}

int pager_fly_map(void *cr3, uintptr_t virtual, size_t size, uint32_t attributes) {
	attributes |= 1 << ARC_PAGER_4K;
	struct pager_traverse_info info = { .virtual = virtual, .size = size, .attributes = attributes, .cr3 = cr3 == NULL ? pml4 : cr3 };

	if (pager_traverse(&info, pager_fly_map_callback) != 0) {
		ARC_DEBUG(ERR, "Failed to fly map 0x%"PRIx64" (0x%"PRIx64" B, 0x%x)\n", virtual, size, attributes);
		return -1;
	}

	return 0;
}

static int pager_fly_unmap_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}

	pmm_free((void *)ARC_PHYS_TO_HHDM(table[index] & ADDRESS_MASK));
	table[index] = 0;

	if (info->cr3 == pml4) {
		__asm__("invlpg [%0]" : : "r"(info->virtual) : );
	}

	return 0;
}

int pager_fly_unmap(void *cr3, uintptr_t virtual, size_t size) {
	struct pager_traverse_info info = { .virtual = virtual, .size = size, .cr3 = cr3 == NULL ? pml4 : cr3 };

	if (pager_traverse(&info, pager_fly_unmap_callback) != 0) {
		ARC_DEBUG(ERR, "Failed to map V0x%"PRIx64" (0x%"PRIx64" B)\n", virtual, size);
		return -1;
	}

	return 0;
}

static int pager_set_attr_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}

	uint64_t address = table[index] & ADDRESS_MASK;
	table[index] = address | get_entry_bits(level, info->attributes);

	if (info->cr3 == pml4) {
		__asm__("invlpg [%0]" : : "r"(info->virtual) : );
	}

	return 0;
}

int pager_set_attr(void *cr3, uintptr_t virtual, size_t size, uint32_t attributes) {
	struct pager_traverse_info info = { .virtual = virtual, .size = size, .attributes = attributes, .cr3 = cr3 == NULL ? pml4 : cr3 };

	if (pager_traverse(&info, pager_set_attr_callback) != 0) {
		ARC_DEBUG(ERR, "Failed to set attr V0x%"PRIx64" (0x%"PRIx64" B, 0x%x)\n", virtual, size, attributes);
		return -1;
	}

	return 0;
}

int init_pager() {
	ARC_DEBUG(INFO, "Initializing pager\n");

	_x86_getCR3();
	pml4 = (uint64_t *)ARC_PHYS_TO_HHDM(_x86_CR3);

	ARC_DEBUG(INFO, "Initialized pager (%p)\n", pml4);

	return 0;
}
