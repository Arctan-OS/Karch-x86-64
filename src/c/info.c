/**
 * @file info.c
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
#include "arch/info.h"
#include "global.h"
#include "lib/hash.h"

#include <cpuid.h>

// Manufacturers
#define AMD           0xc0e946f161868306
#define CENTAUR       0xc3feac365a7e42f0
#define CYRIX         0x3f6ed9c1227d9b72
#define INTEL         0xd4770f6cacd93a6
#define IOTEL         0x7e7850f0c2cebad7
#define TRANSMETA_0   0xf54492b7866dc616
#define TRANSMETA_1   0xf979a96fcfca0465
#define NSC           0x24f8e263d35c64ce
#define NEXGEN        0x70abec693219a8ec
#define RISE          0x4c6136a3bf29e0d8
#define SIS           0xd6e06392fdfa6152
#define UMC           0x52f867d53d99f1f0
#define VORTEX86      0x782d2640fd2d0a64
#define ZHAOXIN       0x8c9acc8e4c2f38bc
#define HYGON         0x4fc576e4f1b66033
#define RDC           0xf34f0a897b4a15d9
#define MCST          0xc313b6467098a08a
#define VIA           0x41955f7c0545e499
#define AMDK5         0xd091667ff54e6365

// Open Source Soft CPUs
#define A0486_0       0xb8bd52c8d63abe64
#define A0486_1       0x23419031df8acf0d
#define V586          INTEL

// Virtual Machines
#define CONNECTIX6    0x93439b04990ce6c2
#define MSVPC7        0x54408e3e4b4e989a
#define MS_x64_to_ARM AMD
#define APPLE_ROSETA  INTEL
#define INSIGNIA      0xa04312039a38470c
#define COMPAQ        0xd3b4c45387b8ef38
#define POWERVM       0x7ef33c15ad13938f
#define NEKO          0xba5650baf46685b

#define DECLARE_REGISTERS32 \
        register uint32_t eax = 0; \
        register uint32_t ebx = 0; \
        register uint32_t ecx = 0; \
        register uint32_t edx = 0; \

uint32_t arch_physical_address_width() {
        DECLARE_REGISTERS32;

        __cpuid(0x80000008, eax, ebx, ecx, edx);

        return MASKED_READ(eax, 0, 0xFF);
}

uint32_t arch_virtual_address_width() {
        DECLARE_REGISTERS32;

        __cpuid(0x80000008, eax, ebx, ecx, edx);

        return MASKED_READ(eax, 8, 0xFF);
}

ARC_ARCHTYPE arch_processor_type() {
        DECLARE_REGISTERS32;
        __cpuid(0, eax, ebx, ecx, edx);

        uint32_t istr[4] = { ebx, edx, ecx, 0 };
        uint64_t hash = hash_fnv1a((uint8_t *)&istr, 12);

        switch(hash) {
                case AMD:
                        return ARC_ARCH_TYPE_AMD;
                case CENTAUR:
                        return ARC_ARCH_TYPE_CENTAUR;
                case CYRIX:
                        return ARC_ARCH_TYPE_CYRIX;
                case INTEL:
                case IOTEL:
                        return ARC_ARCH_TYPE_INTEL;
                case TRANSMETA_0:
                case TRANSMETA_1:
                        return ARC_ARCH_TYPE_TRANSMETA;
                case NSC:
                        return ARC_ARCH_TYPE_NSC;
                case NEXGEN:
                        return ARC_ARCH_TYPE_NEXGEN;
                case RISE:
                        return ARC_ARCH_TYPE_RISE;
                case SIS:
                        return ARC_ARCH_TYPE_SIS;
                case UMC:
                        return ARC_ARCH_TYPE_UMC;
                case VORTEX86:
                        return ARC_ARCH_TYPE_VORTEX86;
                case ZHAOXIN:
                        return ARC_ARCH_TYPE_ZHAOXIN;
                case HYGON:
                        return ARC_ARCH_TYPE_HYGON;
                case RDC:
                        return ARC_ARCH_TYPE_RDC;
                case MCST:
                        return ARC_ARCH_TYPE_MCST;
                case VIA:
                        return ARC_ARCH_TYPE_VIA;
                case AMDK5:
                        return ARC_ARCH_TYPE_AMDK5;
                case A0486_0:
                        return ARC_ARCH_TYPE_A0486_OLD;
                case A0486_1:
                        return ARC_ARCH_TYPE_A0486_NEW;
                case CONNECTIX6:
                        return ARC_ARCH_TYPE_CONNECTIX6;
                case MSVPC7:
                        return ARC_ARCH_TYPE_MSVPC7;
                case INSIGNIA:
                        return ARC_ARCH_TYPE_INSIGNIA;
                case COMPAQ:
                        return ARC_ARCH_TYPE_COMPAQ;
                case POWERVM:
                        return ARC_ARCH_TYPE_POWERVM;
                case NEKO:
                        return ARC_ARCH_TYPE_NEKO;
        }

        return ARC_ARCH_TYPE_MAX;
}

__attribute__((naked)) uint64_t arch_get_cycles() {
	__asm__("rdtsc; shl rdx, 32; or rdx, rax; mov rax, rdx; ret" :::);
}
