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
#include "arch/smp.h"
#include "arch/interrupt.h"
#include "arch/pager.h"
#include "arch/x86-64/apic.h"
#include "arch/x86-64/ctrl_regs.h"
#include "arch/x86-64/context.h"
#include "arch/x86-64/interrupt.h"
#include "global.h"

#define EARLY_KERNEL_CS 0x18

ARC_IDTEntry idt_entries[32] = { 0 };
ARC_IDTRegister idt_register = {
        .base = (uintptr_t)&idt_entries,
        .limit = sizeof(idt_entries) * 16 - 1
};

int init_arch_early() {
        Arc_KernelPageTables = _x86_getCR3();

        internal_init_early_exceptions(idt_entries, EARLY_KERNEL_CS, 0);
        interrupt_load(&idt_register);
        // NOTE: Loading a GDT is not the most vital thing. The bootstrapper should
        //       provide an OK one to use. Only during APIC initialization does a
        //       GDT really need to get created

        return 0;
}

int init_arch() {
        if (init_apic() != 0) {
                ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
                ARC_HANG;
        }

        return 0;
}
