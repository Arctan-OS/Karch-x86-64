/**
 * @file start.c
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
#include "arch/interrupt.h"
#include "arch/pager.h"
#include "arch/x86-64/apic.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/interrupt.h"
#include "arch/x86-64/smp.h"
#include "global.h"
#include "util.h"

#define EARLY_KERNEL_CS 0x18

USERSPACE(data) ARC_x64ProcessorDescriptor bsp = {
        .proc_structs = {
                .idtr = {
                        .base = (uintptr_t)bsp.proc_structs.idt_entries,
                        .limit = (32 * 16) - 1,
                },
        },
};

int init_arch_early() {
        Arc_KernelPageTables = _x86_getCR3();

        memcpy(&bsp.features, &Arc_KernelMeta->features, sizeof(ARC_ProcessorFeatures));
        internal_init_early_exceptions(bsp.proc_structs.idt_entries, EARLY_KERNEL_CS, 0);
        interrupt_load(&bsp.proc_structs.idtr);
        // NOTE: Loading a GDT is not the most vital thing. The bootstrapper should
        //       provide an OK one to use. Only during APIC initialization does a
        //       GDT really need to get created

        context_set_proc_desc(&bsp);

        return 0;
}

int init_arch() {
        if (init_apic() != 0) {
                ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
                ARC_HANG;
        }

        return 0;
}
