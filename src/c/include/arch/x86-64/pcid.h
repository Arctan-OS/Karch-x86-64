#ifndef ARC_ARCH_X86_64_PCID_H
#define ARC_ARCH_X86_64_PCID_H

#define PCID_COUNT 2 << 12

int pcid_allocate();
void pcid_free(int pcid);

int init_pcid();

#endif
