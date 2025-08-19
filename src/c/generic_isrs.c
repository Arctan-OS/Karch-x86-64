/**
 * @file generic_isrs.c
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
*/
#include "arch/x86-64/interrupt.h"
#include "arch/x86-64/ctrl_regs.h"

ARC_IDTEntry idt_entries[32] = { 0 };
ARC_IDTRegister idt_register = {
        .base = (uintptr_t)&idt_entries,
        .limit = sizeof(idt_entries) * 16 - 1
};

static ARC_GenericSpinlock panic_lock;

GENERIC_HANDLER(0) {
	GENERIC_EXCEPTION_PREAMBLE(0);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(0);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(1) {
	GENERIC_EXCEPTION_PREAMBLE(1);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(1);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(2) {
	GENERIC_EXCEPTION_PREAMBLE(2);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(2);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(3) {
	GENERIC_EXCEPTION_PREAMBLE(3);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(3);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(4) {
	GENERIC_EXCEPTION_PREAMBLE(4);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(4);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(5) {
	GENERIC_EXCEPTION_PREAMBLE(5);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(5);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(6) {
	GENERIC_EXCEPTION_PREAMBLE(6);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(6);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(7) {
	GENERIC_EXCEPTION_PREAMBLE(7);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(7);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(8) {
	GENERIC_EXCEPTION_PREAMBLE(8);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(8);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(9) {
	GENERIC_EXCEPTION_PREAMBLE(9);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(9);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(10) {
	GENERIC_EXCEPTION_PREAMBLE(10);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(10);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(11) {
	GENERIC_EXCEPTION_PREAMBLE(11);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(11);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(12) {
	GENERIC_EXCEPTION_PREAMBLE(12);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(12);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(13) {
	GENERIC_EXCEPTION_PREAMBLE(13);
	GENERIC_EXCEPTION_REG_DUMP(13);
	if (interrupt_error_code == 0) {
		printf("#GP may have been caused by one of the following:\n");
		printf("\tAn operand of the instruction\n");
		printf("\tA selector from a gate which is the operand of the instruction\n");
		printf("\tA selector from a TSS involved in a task switch\n");
		printf("\tIDT vector number\n");
	}

	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(14) {
	GENERIC_EXCEPTION_PREAMBLE(14);
        (void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(14);
        printf("CR2: 0x%016"PRIx64"\n", _x86_getCR2());
        printf("CR3: 0x%016"PRIx64"\n", _x86_getCR3());
        spinlock_unlock(&panic_lock);
        ARC_HANG;
}

GENERIC_HANDLER(15) {
	GENERIC_EXCEPTION_PREAMBLE(15);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(15);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(16) {
	GENERIC_EXCEPTION_PREAMBLE(16);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(16);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(17) {
	GENERIC_EXCEPTION_PREAMBLE(17);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(17);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(18) {
	GENERIC_EXCEPTION_PREAMBLE(18);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(18);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(19) {
	GENERIC_EXCEPTION_PREAMBLE(19);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(19);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(20) {
	GENERIC_EXCEPTION_PREAMBLE(20);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(20);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(21) {
	GENERIC_EXCEPTION_PREAMBLE(21);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(21);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(22) {
	GENERIC_EXCEPTION_PREAMBLE(22);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(22);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(23) {
	GENERIC_EXCEPTION_PREAMBLE(23);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(23);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(24) {
	GENERIC_EXCEPTION_PREAMBLE(24);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(24);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(25) {
	GENERIC_EXCEPTION_PREAMBLE(25);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(25);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(26) {
	GENERIC_EXCEPTION_PREAMBLE(26);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(26);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(27) {
	GENERIC_EXCEPTION_PREAMBLE(27);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(27);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(28) {
	GENERIC_EXCEPTION_PREAMBLE(28);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(28);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(29) {
	GENERIC_EXCEPTION_PREAMBLE(29);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(29);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(30) {
	GENERIC_EXCEPTION_PREAMBLE(30);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(30);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

GENERIC_HANDLER(31) {
	GENERIC_EXCEPTION_PREAMBLE(31);
	(void)interrupt_error_code;
        GENERIC_EXCEPTION_REG_DUMP(31);
	spinlock_unlock(&panic_lock);
	ARC_HANG;
}

int internal_init_early_exceptions(ARC_IDTEntry *entries, uint16_t kcode_seg, uint8_t ist) {
	if (entries == NULL) {
		return -1;
	}

	GENERIC_HANDLER_INSTALL(entries, 0, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 1, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 2, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 3, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 4, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 5, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 6, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 7, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 8, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 9, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 10, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 11, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 12, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 13, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 14, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 15, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 16, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 17, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 18, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 19, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 20, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 21, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 22, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 23, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 24, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 25, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 26, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 27, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 28, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 29, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 30, kcode_seg, ist);
	GENERIC_HANDLER_INSTALL(entries, 31, kcode_seg, ist);

        return 0;
}
