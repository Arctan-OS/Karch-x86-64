/**
 * @file info.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Karch - Abstract Definition, Declaration of Architecture Functions
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Karch.
 *
 * Arctan-OS/Karch is free software; you can redistribute it and/or
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
#ifndef ARC_ARCH_X86_64_INFO_H
#define ARC_ARCH_X86_64_INFO_H

enum {
        ARC_RFLAGS_CARRY,
        ARC_RFLAGS_RESV0,
        ARC_RFLAGS_PARITY,
        ARC_RFLAGS_RESV1,
        ARC_RFLAGS_AUX,
        ARC_RFLAGS_RESV2,
        ARC_RFLAGS_ZERO,
        ARC_RFLAGS_SIGN,
        ARC_RFLAGS_TRAP,
        ARC_RFLAGS_IRQ_ENABLE,
        ARC_RLFAGS_DIRECTION,
        ARC_RFLAGS_OVERFLOW,
        ARC_RFLAGS_IOPL,
        ARC_RFLAGS_NESTED,
        ARC_RFLAGS_MODE,
        ARC_RFLAGS_RESUME,
        ARC_RFLAGS_VM8086,
        ARC_RFLAGS_ALIGN_CHK,
        ARC_RFLAGS_VM_INTERRUPT,
        ARC_RFLAGS_CPUID,
        ARC_RFLAGS_RESV3,
        ARC_RFLAGS_AES_KEY_SCHED,
        ARC_RFLAGS_ALT_INSTRUCTIONS,
        ARC_RFLAGS_MAX
};

#endif
