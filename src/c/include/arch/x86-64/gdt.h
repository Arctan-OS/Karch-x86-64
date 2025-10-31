/**
 * @file gdt.h
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
 * Change out the GDT to better suit 64-bit mode, remove no longer needed 32-bit
 * segments.
*/
#ifndef ARC_ARCH_X86_64_GDT_H
#define ARC_ARCH_X86_64_GDT_H

#include <stdint.h>

typedef struct ARC_GDTEntry {
        uint16_t limit;
        uint16_t base1;
        uint8_t base2;
        uint8_t access;
        uint8_t flags_limit;
        uint8_t base3;
} __attribute__((packed)) ARC_GDTEntry;

typedef struct ARC_TSSEntry {
        uint16_t limit;
        uint16_t base1;
        uint8_t base2;
        uint8_t access;
        uint8_t flags_limit;
        uint8_t base3;
        uint32_t base4;
        uint32_t resv;
}__attribute__((packed)) ARC_TSSEntry;

typedef struct ARC_TSSDescriptor {
        uint32_t resv0;
        uint32_t rsp0_low;
        uint32_t rsp0_high;
        uint32_t rsp1_low;
        uint32_t rsp1_high;
        uint32_t rsp2_low;
        uint32_t rsp2_high;
        uint32_t resv1;
        uint32_t resv2;
        uint32_t ist1_low;
        uint32_t ist1_high;
        uint32_t ist2_low;
        uint32_t ist2_high;
        uint32_t ist3_low;
        uint32_t ist3_high;
        uint32_t ist4_low;
        uint32_t ist4_high;
        uint32_t ist5_low;
        uint32_t ist5_high;
        uint32_t ist6_low;
        uint32_t ist6_high;
        uint32_t ist7_low;
        uint32_t ist7_high;
        uint32_t resv3;
        uint32_t resv4;
        uint16_t resv5;
        uint16_t io_port_bmp_off;
}__attribute__((packed)) ARC_TSSDescriptor;

// NOTE: Currently, I think 8 regular GDT entries and one TSS entry is plenty
//       so given this I will keep the GDT register like this so that everything
//       is in one place. This organization allows for the reading of the GDTR
//       and casting it directly to this structure.
typedef struct ARC_GDTRegister {
        ARC_GDTEntry gdt[8];
        ARC_TSSEntry tss;
        struct {
                uint16_t size;
                uint64_t base;
        } __attribute__((packed)) reg;
} __attribute__((packed)) ARC_GDTRegister;

int gdt_load(ARC_GDTRegister *gdtr);
int gdt_use_tss(ARC_GDTRegister *gdtr, ARC_TSSDescriptor *tss);
int init_static_tss(ARC_TSSDescriptor *tss, uintptr_t ist1, uintptr_t rsp0);
ARC_TSSDescriptor *init_tss(uintptr_t ist1, uintptr_t rsp0);
int init_static_gdt(ARC_GDTRegister *gdtr);
ARC_GDTRegister *init_gdt();

#endif
