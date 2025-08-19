/**
 * @file smp.h
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
 * Specific structures and functions for initialization of symmetric multi-processing
 * on x86-64 systems.
*/
#ifndef ARC_ARCH_X86_64_SMP_H
#define ARC_ARCH_X86_64_SMP_H

#include "arch/smp.h"

// TODO: This structure can be changed such that there is no need for syscall_stack
//       by making it so that the KernelGS pointer points to the stack, and on the stack
//       is this structure. This would also eliminate the need for a processor descriptor list.
typedef struct ARC_x64ProcessorDescriptor {
        uintptr_t syscall_stack;
        ARC_ProcessorDescriptor descriptor;
} __attribute__((packed)) ARC_x64ProcessorDescriptor;

// NOTE: The index in Arc_ProcessorList corresponds to the ID
//       acquired from get_processor_id();
extern ARC_x64ProcessorDescriptor *Arc_ProcessorList;
// NOTE: Since the processor structure includes a next pointer,
//       this allows us to also traverse the list by processor,
//       while still being able to address the list by processor ID
//       in O(1) time
extern ARC_x64ProcessorDescriptor *Arc_BootProcessor;

/**
 * Initialize an AP into an SMP system.
 *
 * Initializes the given AP into an SMP system by creating the
 * relevant processor descriptors and sending INIT and START IPIs.
 *
 * NOTE: This functions is meant to only be called from the BSP.
 * NOTE: This function should be called to initialize the BSP as well
 *       since it needs to be inserted into the descriptor list.
 * @param uint32_t lapic - The ID of the LAPIC to initialize.
 * @param uint32_t acpi_uid - The UID given to the processor by ACPI.
 * @param uint32_t acpi_flags - The flags given by ACPI.
 * @param uint32_t version - The version of the LAPIC.
 * @return zero upon success.
 * */
int smp_init_ap(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version);

#endif
