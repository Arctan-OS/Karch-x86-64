/**
 * @file syscall.c
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
#include <global.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/ctrl_regs.h>
#include <stdint.h>

struct ARC_SyscallArgs {
	uint64_t a;
	uint64_t b;
	uint64_t c;
	uint64_t d;
	uint64_t e;
	uint64_t f;
	uint64_t g;
}__attribute__((packed));


static int syscall_0(struct ARC_SyscallArgs *args) {
	(void)args;
	// TCB_SET
	return 0;
}

static int syscall_1(struct ARC_SyscallArgs *args) {
	(void)args;
	// FUTEX_WAIT
	return 0;
}
static int syscall_2(struct ARC_SyscallArgs *args) {
	(void)args;
	// FUTEX_WAKE
	return 0;
}

static int syscall_3(struct ARC_SyscallArgs *args) {
	(void)args;
	// CLOCK_GET
	return 0;
}

static int syscall_4(int code) {
	ARC_DEBUG(INFO, "Exiting (%d)\n", code);
	term_draw(&Arc_MainTerm);
	// EXIT
	return 0;
}

static int syscall_5(struct ARC_SyscallArgs *args) {
	(void)args;
	// SEEK
	return 0;
}
static int syscall_6(struct ARC_SyscallArgs *args) {
	(void)args;
	// WRITE
	return 0;
}

static int syscall_7(struct ARC_SyscallArgs *args) {
	(void)args;
	// READ
	return 0;
}

static int syscall_8(struct ARC_SyscallArgs *args) {
	(void)args;
	// CLOSE
	return 0;
}

static int syscall_9(struct ARC_SyscallArgs *args) {
	(void)args;
	// OPEN
	return 0;
}
static int syscall_A(struct ARC_SyscallArgs *args) {
	(void)args;
	// VM_MAP
	return 0;
}

static int syscall_B(struct ARC_SyscallArgs *args) {
	(void)args;
	// VM_UNMAP
	return 0;
}

static int syscall_C(struct ARC_SyscallArgs *args) {
	(void)args;
	// ANON_ALLOC
	return 0;
}

static int syscall_D(struct ARC_SyscallArgs *args) {
	(void)args;
	// ANON_FREE
	return 0;
}

static int syscall_E(struct ARC_SyscallArgs *args) {
	(void)args;
	// LIBC LOG
	return 0;
}

static int syscall_F(struct ARC_SyscallArgs *args) {
	(void)args;
	return 0;
}

int (*Arc_SyscallTable[])() = {
	syscall_0,
	syscall_1,
	syscall_2,
	syscall_3,
	syscall_4,
	syscall_5,
	syscall_6,
	syscall_7,
	syscall_8,
	syscall_9,
	syscall_A,
	syscall_B,
	syscall_C,
	syscall_D,
	syscall_E,
	syscall_F
};

extern int _syscall(uint64_t code, struct ARC_SyscallArgs *args);
int init_syscall() {
	uint64_t ia32_fmask = 0;
	_x86_WRMSR(0xC0000084, ia32_fmask);

	uint64_t ia32_lstar = _x86_RDMSR(0xC0000082);
	ia32_lstar = (uintptr_t)_syscall;
	_x86_WRMSR(0xC0000082, ia32_lstar);

	uint64_t ia32_star = _x86_RDMSR(0xC0000081);
	uint64_t syscall_ss_cs = 0x08;
	uint32_t syscall_eip = 0x0;
	uint64_t sysret_ss_cs = 0x10; // CS: This value + 0x10, SS: This value + 0x8
	ia32_star |= sysret_ss_cs << 48 | syscall_ss_cs << 32 | syscall_eip;
	_x86_WRMSR(0xC0000081, ia32_star);

	uint64_t ia32_efer = _x86_RDMSR(0xC0000080);
	ia32_efer |= 1;
	_x86_WRMSR(0xC0000080, ia32_efer);

	ARC_DEBUG(INFO, "Installed syscalls\n");
	ARC_DEBUG(INFO, "\tEFER: 0x%lX\n", ia32_efer);
	ARC_DEBUG(INFO, "\tSTAR: 0x%lX\n", ia32_star);
	ARC_DEBUG(INFO, "\tLSTAR: 0x%lX\n", ia32_lstar);

	return 0;
}
