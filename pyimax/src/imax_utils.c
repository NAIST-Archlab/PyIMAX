#include "../include/imax_utils.h"

unsigned char* sysinit(Uint memsize, Uint alignment, Uint threadQty) {
    unsigned char* membase = NULL;
    printf("sysinit: memsize=%d, alignment=%d, threadQty=%d\n", memsize, alignment, threadQty);
#if defined(ARMZYNQ) && defined(EMAX7)
    printf("sysinit: emax7_open(%d)\n", threadQty);
    if (emax7_open(threadQty) == NULL) exit(1);
    membase = emax_info[0].ddr_mmap;
    {int j;for (j = 0; j < (memsize + sizeof(Dll) - 1) / sizeof(Dll); j++)*((Dll *)membase + j) = 0;}
#elif __linux__ == 1
    for (int i = 0; i < threadQty; i++) {
        posix_memalign((void**)&membase, alignment, memsize);
    }
#else
    for (int i = 0; i < threadQty; i++) {
        membase= (void *)malloc(memsize + alignment);
        if ((Ull)membase & (Ull)(alignment - 1)) {membase = (void *)(((Ull)membase & ~(Ull)(alignment - 1)) + alignment);}
    }
#endif

#if !defined(ARMZYNQ) && defined(EMAX7)
    for (int i = 0; i < threadQty; i++) {
        emax_info[i].dma_phys = DMA_BASE2_PHYS; /* defined in emax7lib.h */
        emax_info[i].dma_mmap = emax_info[i].dma_phys;
        emax_info[i].reg_phys = REG_BASE2_PHYS; /* defined in emax7lib.h */
        emax_info[i].reg_mmap = emax_info[i].reg_phys;
        emax_info[i].lmm_phys = LMM_BASE2_PHYS;
        emax_info[i].lmm_mmap = emax_info[i].lmm_phys;
        emax_info[i].ddr_phys = membase[i];
        emax_info[i].ddr_mmap = emax_info[i].ddr_phys;
    }
#endif

#if (defined(ARMSIML) || defined(ARMZYNQ)) && defined(EMAX7)
    for (int i = 0; i < threadQty; i++) {
        emax7[i].dma_ctrl = emax_info[i].dma_mmap;
        emax7[i].reg_ctrl = emax_info[i].reg_mmap;
        ((struct reg_ctrl *)emax7[i].reg_ctrl)->i[i].cmd = CMD_RESET;
#if defined(ARMZYNQ)
        usleep(1);
#endif
        switch (((struct reg_ctrl *)emax7[i].reg_ctrl)->i[i].stat >> 8 & 0xf) {
        case 3:
            EMAX_DEPTH = 64;
            break;
        case 2:
            EMAX_DEPTH = 32;
            break;
        case 1:
            EMAX_DEPTH = 16;
            break;
        default:
            EMAX_DEPTH = 8;
            break;
        }
        ((struct reg_ctrl *)emax7[i].reg_ctrl)->i[i].adtr = emax_info[i].ddr_mmap - emax_info[i].lmm_phys;
        ((struct reg_ctrl *)emax7[i].reg_ctrl)->i[i].dmrp = 0LL;
    }
#endif
    return membase;
}

void imemcpy(Uint *dst, Uint *src, int words) {
    union {
        Uint i[4];
        Ull l[2];
        Dll d;
    } buf;

    Uint loop, i;
    if (words >= 1 && ((Ull)dst & sizeof(Uint))) { /* 4B-access odd */
        *dst++ = *src++;
        words--;
    }
    if (words >= 2 && ((Ull)dst & sizeof(Ull))) { /* 8B-access odd */
        if ((Ull)src & sizeof(Uint)) {
            buf.i[0] = *src++;
            buf.i[1] = *src++;
            *(Ull *)dst = buf.l[0];
        } else {
            *(Ull *)dst = *(Ull *)src;
            src += sizeof(Ull) / sizeof(Uint);
        }
        dst += sizeof(Ull) / sizeof(Uint);
        words -= 2;
    }

    if (loop = words / (sizeof(Dll) / sizeof(Uint))) {
        if ((Ull)src & sizeof(Uint)) {
            for (i = 0; i < loop; i++) {
                buf.i[0] = *src++;
                buf.i[1] = *src++;
                buf.i[2] = *src++;
                buf.i[3] = *src++;
                *(Dll *)dst = buf.d;
                dst += sizeof(Dll) / sizeof(Uint);
            }
        } else if ((Ull)src & sizeof(Ull)) {
            for (i = 0; i < loop; i++) {
                buf.l[0] = *(Ull *)src;
                src += sizeof(Ull) / sizeof(Uint);
                buf.l[1] = *(Ull *)src;
                src += sizeof(Ull) / sizeof(Uint);
                *(Dll *)dst = buf.d;
                dst += sizeof(Dll) / sizeof(Uint);
            }
        } else {
            for (i = 0; i < loop; i++) {
                *(Dll *)dst = *(Dll *)src;
                src += sizeof(Dll) / sizeof(Uint);
                dst += sizeof(Dll) / sizeof(Uint);
            }
        }
        words -= loop * (sizeof(Dll) / sizeof(Uint));
    }

    if (words >= 2) { /* 8B-access */
        if ((Ull)src & sizeof(Uint)) {
            buf.i[0] = *src++;
            buf.i[1] = *src++;
            *(Ull *)dst = buf.l[0];
        } else {
            *(Ull *)dst = *(Ull *)src;
            src += sizeof(Ull) / sizeof(Uint);
        }
        dst += sizeof(Ull) / sizeof(Uint);
        words -= 2;
    }
    if (words >= 1) { /* 4B-access */
        *dst++ = *src++;
        words--;
    }
}

void xmax_bzero(Uint *dst, int words) {
    Uint loop, i;
    if (words >= 1 && ((Ull)dst & sizeof(Uint))) { /* 4B-access odd */
        *dst++ = 0;
        words--;
    }
    if (words >= 2 && ((Ull)dst & sizeof(Ull))) { /* 8B-access odd */
        *(Ull *)dst = 0;
        dst += sizeof(Ull) / sizeof(Uint);
        words -= 2;
    }

    if (loop = words / (sizeof(Dll) / sizeof(Uint))) {
        for (i = 0; i < loop; i++) {
#if __AARCH64EL__ == 1
            *((Dll *)dst) = 0;
#else
            ((Dll *)dst)->u[0] = 0;
            ((Dll *)dst)->u[1] = 0;
#endif
            dst += sizeof(Dll) / sizeof(Uint);
        }
        words -= loop * (sizeof(Dll) / sizeof(Uint));
    }

    if (words >= 2) { /* 8B-access */
        *(Ull *)dst = 0;
        dst += sizeof(Ull) / sizeof(Uint);
        words -= 2;
    }
    if (words >= 1) { /* 4B-access */
        *dst++ = 0;
        words--;
    }
}