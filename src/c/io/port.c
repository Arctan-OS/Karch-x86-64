#include <arch/x86-64/io/port.h>

void outw(uint16_t port, uint16_t value) {
	outb(port++, ((value >> 0) & 0xFF));
	outb(port, ((value >> 8) & 0xFF));
}

uint16_t inw(uint16_t port) {
	uint16_t res = 0;

	res |= (uint16_t)inb(port++) << 0;
	res |= (uint16_t)inb(port) << 8;

	return res;
}

void outd(uint16_t port, uint32_t value) {
	outb(port++, ((value >> 0) & 0xFF));
	outb(port++, ((value >> 8) & 0xFF));
	outb(port++, ((value >> 16) & 0xFF));
	outb(port, ((value >> 24) & 0xFF));
}

uint32_t ind(uint16_t port) {
	uint32_t res = 0;

	res |= (uint32_t)inb(port++) << 0;
	res |= (uint32_t)inb(port++) << 8;
	res |= (uint32_t)inb(port++) << 16;
	res |= (uint32_t)inb(port) << 24;

	return res;
}

void outq(uint16_t port, uint64_t value) {
	outb(port++, ((value >> 0) & 0xFF));
	outb(port++, ((value >> 8) & 0xFF));
	outb(port++, ((value >> 16) & 0xFF));
	outb(port++, ((value >> 24) & 0xFF));
	outb(port++, ((value >> 32) & 0xFF));
	outb(port++, ((value >> 40) & 0xFF));
	outb(port++, ((value >> 48) & 0xFF));
	outb(port, ((value >> 56) & 0xFF));
}

uint64_t inq(uint16_t port) {
	uint64_t res = 0;

	res |= (uint64_t)inb(port++) << 0;
	res |= (uint64_t)inb(port++) << 8;
	res |= (uint64_t)inb(port++) << 16;
	res |= (uint64_t)inb(port++) << 24;
	res |= (uint64_t)inb(port++) << 32;
	res |= (uint64_t)inb(port++) << 40;
	res |= (uint64_t)inb(port++) << 48;
	res |= (uint64_t)inb(port) << 56;

	return res;
}
