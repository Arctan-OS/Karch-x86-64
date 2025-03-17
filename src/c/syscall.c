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
#include <mp/scheduler.h>
#include <arch/smp.h>
#include <stdint.h>

static int syscall_tcb_set(void *arg) {
	(void)arg;

	printf("TCB Set\n");
	// TCB_SET
	return 0;
}

static int syscall_futex_wait(int *ptr, int expected, struct timespec const *time) {
	(void)ptr;
	(void)expected;
	(void)time;

	printf("Futex wait\n");
	// FUTEX_WAIT
	return 0;
}
static int syscall_futex_wake(int *ptr) {
	(void)ptr;

	printf("Futex wake\n");
	// FUTEX_WAKE
	return 0;
}

static int syscall_clock_get(int a, long *b, long *c) {
	(void)a;
	(void)b;
	(void)c;

	printf("Syscall clock get\n");
	// CLOCK_GET
	return 0;
}

static int syscall_exit(int code) {
	(void)code;

	ARC_DEBUG(INFO, "Exiting %d\n", code);
	struct ARC_ProcessorDescriptor *desc = smp_get_proc_desc();
	sched_dequeue(desc->current_process);
	
	return 0;
}

static int syscall_seek(int fd, long offset, int whence, long *new_offset) {
	(void)fd;
	(void)offset;
	(void)whence;
	(void)new_offset;

	printf("Seek\n");
	// SEEK
	return 0;
}

static int syscall_write(int fd, void const *a, unsigned long b, long *c) {
	(void)fd;
	(void)a;
	(void)b;
	(void)c;

	printf("Writing\n");
	// WRITE
	return 0;
}

static int syscall_read(int fd, void *buf, unsigned long count, long *bytes_read) {
	(void)fd;
	(void)buf;
	(void)count;
	(void)bytes_read;

	printf("Reading\n");
	// READ
	return 0;
}

static int syscall_close(int fd) {
	(void)fd;

	printf("Closing\n");
	// CLOSE
	return 0;
}

static int syscall_open(char const *name, int flags, unsigned int mode, int *fd) {
	(void)name;
	(void)flags;
	(void)mode;
	(void)fd;

	printf("Opening\n");
	// OPEN
	return 0;
}
static int syscall_vm_map(void *hint, unsigned long size, int prot, int flags, int fd, long offset, void **window) {
	(void)hint;
	(void)size;
	(void)prot;
	(void)flags;
	(void)fd;
	(void)offset;
	(void)window;

	printf("Mapping\n");
	// VM_MAP
	return 0;
}

static int syscall_vm_unmap(void *a, unsigned long b) {
	(void)a;
	(void)b;

	printf("Unmapping\n");
	// VM_UNMAP
	return 0;
}

static int syscall_anon_alloc(unsigned long size, void **ptr) {
	(void)size;
	(void)ptr;

	printf("Allocating\n");
	// ANON_ALLOC
	return 0;
}

static int syscall_anon_free(void *ptr, unsigned long size) {
	(void)ptr;
	(void)size;

	printf("Freeing\n");
	// ANON_FREE
	return 0;
}

static int syscall_libc_log(const char *str) {
	(void)str;
	
	// LIBC LOG
	printf("%s\n", str);
	return 0;
}

int (*Arc_SyscallTable[])() = {
	[0] = syscall_tcb_set,
	[1] = syscall_futex_wait,
	[2] = syscall_futex_wake,
	[3] = syscall_clock_get,
	[4] = syscall_exit,
	[5] = syscall_seek,
	[6] = syscall_write,
	[7] = syscall_read,
	[8] = syscall_close,
	[9] = syscall_open,
	[10] = syscall_vm_map,
	[11] = syscall_vm_unmap,
	[12] = syscall_anon_alloc,
	[13] = syscall_anon_free,
	[14] = syscall_libc_log,
};

extern int _syscall();
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
