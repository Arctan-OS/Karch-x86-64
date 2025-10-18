#include "arch/x86-64/pcid.h"
#include "arch/x86-64/smp.h"
#include "arctan.h"
#include "global.h"
#include "mm/allocator.h"
#include "util.h"

int pcid_allocate() {
        uint64_t *pcid_bmp = Arc_CurProcessorDescriptor->pcid.bmp;

        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "No PCID bitmap present, cannot allocate\n");
                return -1;
        }

        int pcid_last_free = Arc_CurProcessorDescriptor->pcid.last_free;
        int pcid = 0;

        for (int i = 0; i < PCID_COUNT / (sizeof(*pcid_bmp) * 8); i++) {
                int _i = (i + pcid_last_free) % PCID_COUNT / 8;

                if (pcid_bmp[_i] == (uint64_t)~0) {
                        continue;
                }

                int x = __builtin_ffs(~pcid_bmp[_i]);

                if (x == 0) {
                        continue;
                }

                x--;

                pcid_bmp[_i] |= 1 << x;
                pcid = (i * sizeof(*pcid_bmp) * 8) + x;
                pcid_last_free = _i;

                break;
        }

        Arc_CurProcessorDescriptor->pcid.last_free = pcid_last_free;

        return pcid;
}

void pcid_free(int pcid) {
        uint64_t *pcid_bmp = Arc_CurProcessorDescriptor->pcid.bmp;

        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "No PCID bitmap present, cannot free\n");
                return;
        }

        pcid_bmp[pcid / (sizeof(*pcid_bmp) * 8)] &= ~(1 << (pcid % 64));
}

int init_pcid() {
        if (!MASKED_READ(Arc_KernelMeta->paging_features, ARC_PAGER_FLAG_PCID, 1)) {
                ARC_DEBUG(ERR, "PCIDs are disabled\n");
                return -1;
        }

        uint64_t *pcid_bmp = alloc(PCID_COUNT / sizeof(*pcid_bmp) * 8);

        if (pcid_bmp == NULL) {
                ARC_DEBUG(ERR, "Failed to allocate memory for PCID bmp\n");
                return -1;
        }

        memset(pcid_bmp, 0, PCID_COUNT / (sizeof(*pcid_bmp) * 8));

        pcid_bmp[0] |= 1;

        Arc_CurProcessorDescriptor->pcid.bmp = pcid_bmp;
        Arc_CurProcessorDescriptor->pcid.last_free = 0;

        return 0;
}
