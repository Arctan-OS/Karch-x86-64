/**
 * @file smp.h
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
 * A header file contianing functions for managing and initializing application processors
 * for symmetric multi-processing specifically on x86-64.
*/
#ifndef ARC_ARCH_X86_64_SMP_H
#define ARC_ARCH_X86_64_SMP_H

#include <stdint.h>
#include <arch/x86-64/context.h>
#include <lib/atomics.h>
#include <stdarg.h>
#include <arch/generic/smp.h>

struct ARC_ProcessorDescriptor {
	uint32_t bist;
	uint32_t model_info;
	struct ARC_GenericProcessorDescriptor generic;
};

/**
 * Hold the given processor.
 * */
int smp_hold(struct ARC_ProcessorDescriptor *processor);

/**
 * Write the given context to the given processor.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_write(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);
/**
 * Write the processor's context to the given registers structure.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_save(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);

/**
 * Tell the processor to execute the given function with the given arguments.
 *
 * NOTE: When a processor accepts the changes, it will write its current register
 *       (prior to accepting the changes) to processor->registers. The caller should
 *       save this if it wishes to return to this state.
 *
 * @param struct ARC_ProcessorDescriptor *processor - The processor to execute the jump on.
 * @param void *function - The function to jump to.
 * @param int argc - The number of arguments given to the function.
 * @param ... - The arguments to pass to the function.
 * @return zero on success.
 * */
int smp_jmp(struct ARC_ProcessorDescriptor *processor, void *function, int argc, ...);

/**
 * Tell the processor to execute a far jump to the given function with the given arguments.
 *
 * NOTE: When a processor accepts the changes, it will write its current register
 *       (prior to accepting the changes) to processor->registers. The caller should
 *       save this if it wishes to return to this state.
 *
 * @param struct ARC_ProcessorDescriptor *processor - The processor to execute the jump on.
 * @parma uint32_t cs - The code segment to jump to.
 * @param void *function - The function to jump to.
 * @param int argc - The number of arguments given to the function.
 * @param ... - The arguments to pass to the function.
 * @return zero on success.
 * */
int smp_far_jmp(struct ARC_ProcessorDescriptor *processor, uint32_t cs, void *function, int argc, ...);

/**
 * List application processors.
 * */
int smp_list_aps();

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
int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version);

#endif
