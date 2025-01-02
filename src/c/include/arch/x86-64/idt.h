/**
 * @file idt.h
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
 * The file which handles the 64-bit IDT.
*/
#ifndef ARC_ARCH_X86_64_IDT_H
#define ARC_ARCH_X86_64_IDT_H

#include <stdint.h>

/**
 * Load the IDTR.
 * */
extern void _install_idt();

/**
 * Set a gate in the IDT
 *
 * The following function sets interrupt number i to be handled by the function located
 * in segment:offset, and sets the given attributes.
 * NOTE: All interrupts use IST1.
 *
 * @param int i - The interrupt's vector.
 * @param uint64_t offset - The offset of the handler function.
 * @parma uint16_t segment - The segment in which the handler function is located.
 * @param uint8_t attrs - The attributes of the interrupt.
 * */
void install_idt_gate(int i, uint64_t offset, uint16_t segment, uint8_t attrs);


/**
 * Initialize the IDT.
 *
 * Create the IDT with handlers for all exceptions and a few IRQs.
 * */
void init_idt();

#endif
