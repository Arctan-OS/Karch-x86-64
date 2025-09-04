/**
 * @file lapic.h
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
#ifndef ARC_ARCH_X86_64_APIC_LOCAL_H
#define ARC_ARCH_X86_64_APIC_LOCAL_H

#include <stdint.h>

#define ARC_LAPIC_IPI_FIXED    (0b000 << 8)
#define ARC_LAPIC_IPI_LWST_PRI (0b001 << 8)
#define ARC_LAPIC_IPI_SMI      (0b010 << 8)
#define ARC_LAPIC_IPI_NMI      (0b100 << 8)
#define ARC_LAPIC_IPI_INIT     (0b101 << 8)
#define ARC_LAPIC_IPI_START    (0b110 << 8)
#define ARC_LAPIC_IPI_PHYSICAL (0b0   << 11)
#define ARC_LAPIC_IPI_LOGICAL  (0b1   << 11)
#define ARC_LAPIC_IPI_DEASRT   (0b0   << 14)
#define ARC_LAPIC_IPI_ASSERT   (0b1   << 14)
#define ARC_LAPIC_IPI_EDGE     (0b0   << 15)
#define ARC_LAPIC_IPI_LEVEL    (0b1   << 15)
#define ARC_LAPIC_IPI_SELF     (0b01  << 18)
#define ARC_LAPIC_IPI_ALLEXC   (0b10  << 18)
#define ARC_LAPIC_IPI_ALLINC   (0b11  << 18)

#define ARC_LAPIC_TIMER_ONESHOT  0b00
#define ARC_LAPIC_TIMER_PERIODIC 0b01
#define ARC_LAPIC_TIMER_TSC      0b10

int lapic_ipi_poll();
int lapic_get_id();
int lapic_calibrate_timer();
void lapic_eoi();
void lapic_ipi(uint8_t vector, uint8_t destination, uint32_t flags);
void lapic_setup_timer(uint8_t vector, uint8_t mode);
void lapic_timer_mask(uint8_t mask);
void lapic_refresh_timer(uint32_t count);
void lapic_divide_timer(uint8_t division);

/*
 * This header contains functions which manage the
 * LAPIC
 * */

int init_lapic();

#endif
