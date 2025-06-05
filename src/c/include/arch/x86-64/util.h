/**
 * @file util.h
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
#ifndef ARC_ARCH_X86_64_UTIL_H
#define ARC_ARCH_X86_64_UTIL_H

#include <interface/terminal.h>

#define ARC_HALT __asm__("hlt");
#define ARC_HANG term_draw(Arc_CurrentTerm); __asm__("1: hlt; jmp 1b");
#define ARC_DISABLE_INTERRUPT __asm__("cli");
#define ARC_ENABLE_INTERRUPT __asm__("sti");

#endif