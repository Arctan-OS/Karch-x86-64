/**
 * @file pcid.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Karch-x86-64 - x86-64 Implementation of K/arch Abstractions
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Karch-x86-64.
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
*/
#include "arch/info.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/pcid.h"
#include "arch/x86-64/smp.h"
#include "arch/x86-64/util.h"
#include "arctan.h"
#include "global.h"
#include "lib/spinlock.h"
#include "mm/allocator.h"
#include "util.h"

static uint64_t *pcid_bmp = NULL;
static int pcid_last_free = 0;
static ARC_Spinlock mod_lock;

int pcid_allocate() {
        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "No PCID bitmap present, cannot allocate\n");
                return -1;
        }

        int pcid = 0;

        for (int i = 0; i < PCID_COUNT / (sizeof(*pcid_bmp) * 8); i++) {
                int _i = (i + pcid_last_free) % PCID_COUNT / 8;

                if (pcid_bmp[_i] == (uint64_t)~0) {
                        continue;
                }

                spinlock_lock(&mod_lock);

                int x = __builtin_ffs(~pcid_bmp[_i]);

                if (x == 0) {
                        continue;
                }

                x--;

                pcid_bmp[_i] |= 1 << x;
                pcid = (i * sizeof(*pcid_bmp) * 8) + x;
                pcid_last_free = _i;

                spinlock_unlock(&mod_lock);

                break;
        }

        return pcid;
}

void pcid_free(int pcid) {
        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "No PCID bitmap present, cannot free\n");
                return;
        }

        spinlock_lock(&mod_lock);
        pcid_bmp[pcid / (sizeof(*pcid_bmp) * 8)] &= ~(1 << (pcid % 64));
        spinlock_unlock(&mod_lock);
}

int init_pcid() {
        if (!MASKED_READ(Arc_CurProcessorDescriptor->features.paging, ARC_PAGER_FLAG_PCID, 1)) {
                ARC_DEBUG(ERR, "PCIDs are disabled\n");
                return -1;
        }

        if (pcid_bmp != NULL) {
                return 0;
        }

        init_static_spinlock(&mod_lock);

        pcid_bmp = alloc(PCID_COUNT / sizeof(*pcid_bmp) * 8);

        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate memory for PCID bmp\n");
                return -1;
        }

        memset(pcid_bmp, 0, PCID_COUNT / (sizeof(*pcid_bmp) * 8));

        pcid_bmp[0] |= 1;

        ARC_DEBUG(INFO, "Initialized PCID allocator\n");

        return 0;
}
