
/* EMAX7 library                        */
/*         Copyright (C) 2013- by NAIST */
/*          Primary writer: Y.Nakashima */
/*                 nakashim@is.naist.jp */

/*******************************************************************************/
/******************************** Defs *****************************************/
/*******************************************************************************/
#pragma once
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <signal.h>
#include "emax7.h"

enum { NANOS_ARM, NANOS_DRAIN, NANOS_CONF, NANOS_REGV, NANOS_RANGE, NANOS_LOAD, NANOS_EXEC, NANOS_TOTAL, NANOS_CLASS };

typedef struct {
  Uint  f : 23;
  Uint  e :  8;
  Uint  s :  1;
} f32bit;

typedef struct {
  Uint  e :  6;
  Uint  b :  1;
  Uint  s :  1; /* lower 8bit */
  Uint dm : 24; /* dummy for >gcc9 */
} wu7bit;

typedef struct {
  Uint  e :  7;
  Uint  s :  1; /* lower 8bit */
  Uint dm : 24; /* dummy for >gcc9 */
} wu8bit;

typedef struct {
  Uchar u[8];
} u64bit;

#define abs(a)    ((a)>  0 ? (a) :-(a)    )
#define ad(a,b)   ((a)<(b)?(b)-(a):(a)-(b))
#define ss(a,b)   ((a)<(b)?   0   :(a)-(b))

/* dma_ctrl_space */
/* https://www.xilinx.com/content/dam/xilinx/support/documents/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf */
struct dma_ctrl {
  /*   Register Name		   Address	Width	Type	Reset Value	Description */
  Uint MM2S_DMACR;             /* 0x00000000��  32      mixed   0x00010000      DMA Control register */
	/*   Field Name    Bits  Type Default Val  Description            */
	/*   Run/Stop         0  rw   0x0   ��	   0:stop-DMA,1:start-DMA */
	/*   Reserved 	      1  ro   0x1	   Reserved for future use */
        /*   Reset            2  rw   0x0          0:normal, 1:reset in progress */
        /*   Keyhole          3  rw   0x0          0:normal, 1:non-incrementing addr */
        /*   Cycle BD En      4  rw   0x0          0:normal, 1:Cycle Buffer Descriptor */
        /*   Reserved      11-5  ro   0x0          Reserved for future use */
        /*   IOC_IrqEn       12  rw   0x0          0:IOC   Intr. disabled, 1:IOC   Intr. enabled */
        /*   Dly_IrqEn       13  rw   0x0          0:Delay Intr. disabled, 1:Delay Intr. enabled */
        /*   Err_IrqEn       14  rw   0x0          0:Error Intr. disabled, 1:Error Intr. enabled */
        /*   Reserved        15  ro   0x0          Reserved for future use */
        /*   IRQThreshold 23-16  rw   0x1          Intr. threshold */
        /*   IRQThreshold 31-24  rw   0x0          Intr. delay time out */

  Uint MM2S_DMASR;             /* 0x00000004��  32      mixed   0x00010000      DMA Status register */
	/*   Field Name    Bits  Type Default Val  Description            */
	/*   Halted           0  ro   0x1   ��     0:DMA channel running, 1:DMA channel halted */
	/*   Idle             1  ro   0x0   ��     0:not Idle, 1:Idle */
        /*   Reserved         2  ro   0x0          Reserved for future use */
        /*   SGIncld          3  ro   C_INCLUDE_SG 0:SG N.A, 1:SG enabled */
        /*   DMAIntErr        4  ro   0x0   ��     0:no DMA internal error, 1:DMA internal error */
        /*   DMASlvErr        5  ro   0x0   ��     0:no DMA slave errors,   1:DMA slave error */
        /*   DMADecErr        6  ro   0x0   ��     0:no DMA decode errors,  1:DMA decode error (invalid address) */
        /*   Reserved         7  ro   0x0          Reserved for future use */
        /*   SGIntErr         8  ro   0x0          0:no SG internal error,  1:SG internal error */
        /*   SGSlvErr         9  ro   0x0          0:no SG slave errors,    1:SG slave error */
        /*   SGDecErr        10  ro   0x0          0:no SG decode errors,   1:SG decode error (invalid address) */
        /*   Reserved        11  ro   0x0          Reserved for future use */
        /*   IOC_Irq         12  rwc  0x0          0:no IOC intr.   1:IOC intr. */
        /*   Dly_Irq         13  rwc  0x0          0:no Delay intr. 1:Delay intr. */
        /*   Err_Irq         14  rwc  0x0          0:no Err intr.   1:Err intr. */
        /*   Reserved        15  ro   0x0          Reserved for future use */
        /*   IRQThreshold 23-16  ro   0x1          Intr. threshold stat */
        /*   IRQThreshold 31-24  ro   0x0          Intr. delay time out stat */

  Uint reserved0[4];           /* 08h - 14h Reserved N/A */
  Uint MM2S_SA;                /* 0x00000018    32      rw      0x00000000      Source Address. Lower 32 bits of address.*/
  Uint MM2S_SA_MSB;            /* 0x0000001c    32      rw      0x00000000      Source Address. Upper 32 bits of address.*/
  Uint reserved1[2];           /* 20h - 24h Reserved N/A */
  Uint MM2S_LENGTH;            /* 0x00000028    32      rw      0x00000000      Transfer Length (Bytes) */
  Uint reserved2[1];           /* 2ch       Reserved N/A */
  Uint S2MM_DMACR;             /* 0x00000030��  32      mixed   0x00010000      DMA Control register */
  Uint S2MM_DMASR;             /* 0x00000034��  32      mixed   0x00010000      DMA Status register */
  Uint reserves3[4];           /* 38h - 44h Reserved N/A */
  Uint S2MM_DA;                /* 0x00000048    32      rw      0x00000000      Destination Address. Lower 32 bit address.*/
  Uint S2MM_DA_MSB;            /* 0x0000004c    32      rw      0x00000000      Destination Address. Upper 32 bit address.*/
  Uint reserved4[2];           /* 50h - 54h Reserved N/A */
  Uint S2MM_LENGTH;            /* 0x00000058    32      rw      0x00000000      Buffer Length (Bytes) */

  /* Simple Mode */
  /* 0. MM2S_DMASR.Halted=0,Idle=1���ǧ */
  /* 1. Start the MM2S channel (MM2S_DMACR.RS = 1).*/
  /* 2. Write a valid source address to MM2S_SA+MM2S_SA_MSB register.*/
  /* 3. Write the bytes to MM2S_LENGTH register. A value of zero has no effect.*/

  /* 0. S2MM_DMASR.Halted=0,Idle=1���ǧ */
  /* 1. Start the S2MM channel (S2MM_DMACR.RS = 1).*/
  /* 2. Write a valid destination address to S2MM_DA+S2MM_DA_MSB register.*/
  /* 3. Write the bytes to S2MM_LENGTH register. A value of zero has no effect.*/

  /* 4. MM2S_DMASR.bit4-6!=0�ʤ饨�顼 */
  /* 4. S2MM_DMASR.bit4-6!=0�ʤ饨�顼 */
  /* 4. MM2S_DMASR.IOC_Irq��1�ˤʤ�ޤ��Ե�,1��񤤤�reset */
  /* 4. S2MM_DMASR.IOC_Irq��1�ˤʤ�ޤ��Ե�,1��񤤤�reset */
};

/* reg_ctrl */
enum { EXRING_IDLE, EXRING_BUSY};
enum { LMRING_IDLE, LMRING_BUSY};
enum { CMD_NOP, CMD_RESET, CMD_SCON, CMD_EXEC};
struct reg_ctrl {
  struct i0 {
    Ull  stat; /* +0000 bit15-12:LMM_SIZE, bit11-8:EMAX_DEPTH, bit7-4:LMRING, bit3-0:EXRING */
    Uint mcid; /* +0008 maximum chip-ID of IMAX (<EMAX_NCHIP) to be chained (activated) */
    Uint dmy0;
    Uint cmd;  /* +0010 host writes Ull cmd then chip# is propagated to succesors */
  /*Uint cid;*//* +0012 chip# ( set by write to cmd ) */
    Uint dmy1;
    Ull  dmy2;
    Ull  adtr; /* +0020 */
    Ull  dmy3;
    Ull  csel; /* +0030 */
    Ull  dmrp; /* +0038 DMAREAD-PREF */
    Ull  dmy4[1016];
    struct conf                    conf[AMAP_DEPTH][EMAX_WIDTH];  /* +2000-3fff */
    struct {Ull  br[UNIT_WIDTH];}  breg[AMAP_DEPTH][EMAX_WIDTH];  /* +4000-5fff *//* unit[cid][EMAX_DEPTH].breg[x][EMAX_WIDTH].br[UNIT_WIDTH] is used */
    struct {Uint ea0b ; /* ea0 base   (for avoiding ld-mask-st, should be extended to 32bits (lower 18bit is available)) */
      /*Ull  dmy0 :14;*/
        Uint ea0o ; /* ea0 offset (for avoiding ld-mask-st, should be extended to 32bits (lower 18bit is available)) */
      /*Ull  dmy1 :14;*/
        Uint ea1b ; /* ea1 base   (for avoiding ld-mask-st, should be extended to 32bits (lower 18bit is available)) */
      /*Ull  dmy2 :14;*/
        Uint ea1o ; /* ea1 offset (for avoiding ld-mask-st, should be extended to 32bits (lower 18bit is available)) */
      /*Ull  dmy3 :14;*/
        Uint top  ; /* LMM-top virtual-address */
      /*Ull  dmy4 : 1;*/
        Uint bot  ; /* LMM-bot virtual-address */
      /*Ull  dmy5 : 1;*/
        Ull  dmy6 ;}           addr[AMAP_DEPTH][EMAX_WIDTH];  /* +6000-7fff */
    struct {Ull  reg[UNIT_WIDTH];} lddmrw[AMAP_DEPTH][EMAX_WIDTH];/* +8000-9fff *//* lddmq/trans-r,lddmq-w */
    Ull dmy5[3072]; /* +a000-ffff */
  } i[EMAX_NCHIP]; /* 0000-ffff */
};

/* emax7 host control */
enum { STATUS_IDLE, STATUS_CONF, STATUS_SCON, STATUS_REGV, STATUS_RANGE, STATUS_DRAIN, STATUS_LOAD, STATUS_START, STATUS_EXEC, STATUS_TERM };


struct emax7 { /* host status of EMAX7 */
  volatile Ull   dma_ctrl;  /* struct dma_ctrl *dma_ctrl  DMA control */
  volatile Ull   reg_ctrl;  /* struct reg_ctrl *reg_ctrl  REG control */

  Ull   status            : 4;
  Ull   csel_save         : 2;
  Ull   last_conf            ; /* for insn_reuse */
  Ull   lmmic             : 1; /* 0:lmm[0] is curent, 1:lmm[1] is current */
  Ull   lmmio             : 1; /* 0:lmm[0] is prev,   1:lmm[1] is prev    */
  Ull   mapdist           : 6; /* specified mapdist */
  Ull   lastdist          : 6; /* lastdist for DYNAMIC_SCON */
  struct lmmi lmmi[EMAX_NCHIP][AMAP_DEPTH][EMAX_WIDTH][2]; /* lmmi for host (len/ofs/top are resolved) */
  Ull   lmmi_bitmap[EMAX_WIDTH];      /* based on lmmi[*][EMAX_WIDTH][2].v */
  Uchar lmmd[AMAP_DEPTH][EMAX_WIDTH]; /* chip#7,6,..,0:clean, 1:dirty, exec��store�ս��1, drainľ��0 */

#ifndef IGNORE_LMMI_BLKGATHER
  Ull   plist                ; /* pointer-list */
  Ull   blkcount          : 7; /* active block number */
  Ull   blksize           : 9; /* 1:64 2:128 3:256 dwords */
  Ull   lmmblktop            ; /* LMM-addr     for LDRQ(blk>0) */
  Ull   lmmblklen            ; /* total dwords for LDRQ(blk>0) */
#endif

  Ull   rw                    ; /* 0:load(mem->lmm), 1:store(lmm->mem)      */
  Ull   ddraddr               ; /* ddr-address                              */
  Ull   lmmaddr               ; /* lmm-address                              */
  Ull   dmalen                ; /* dma-length                               */
  Ull   sigwait               ; /* 0:no macropipe+sigwait, 1:macropipe+sigwait */
  int   *sigstat              ; /* ->th_args.stat (0:idle, 1:run, 2:wait)   */
  sigset_t *sigset            ; /* for sigmask/sigwait                      */

#ifndef IGNORE_LDDMQ_HANDSHAKE
  Ull   fsm_busy          : 1; /* for LDDMQ and TR */
  Ull   lmwd_valid        : 1; /* for LDDMQ */
  Ull   tcureg_valid      : 1; /* fsm->ARM   0 -> 1 -> 1 -> 0 -> 0 -> 0                              */
  Ull   tcureg_ready      : 1; /* fsm<-ARM   0 -> 0 -> 1 -> 0 -> 0 -> 0                              */
  Ull   tcureg_last       : 1; /* fsm->ARM   0 -> 0 -> 0 -> 1 -> 1 -> 0                              */
  Ull   tcureg_term       : 1; /* fsm<-ARM   0 -> 0 -> 0 -> 0 -> 1 -> 0                              */
  Ull   tcureg[UNIT_WIDTH]   ; /* tcu-data        of tcu                       v                     */
                               /* from ARM:  svc 0x1010 ... tcureg_valid->x0                         */
                               /* from ARM:  svc 0x1011 ... 1->tcureg_ready                          */
                               /* from ARM:  svc 0x1012 ... tcureg_last->x0                          */
                               /* from ARM:  svc 0x1013 ... 1->tcureg_term                           */
                               /* from ARM:  svc 0x1014 ... tcureg[3:0]->x3,2,1,0                    */
#endif
} emax7[EMAX_NLANE];

volatile struct emax_info {
  Ull  dma_phys;     // kern-phys
  Ull  dma_vadr;     // not used
  Ull  dma_mmap;     // user-virt Contiguous 64K register space
  Ull  reg_phys;     // kern-phys
  Ull  reg_vadr;     // not used
  Ull  reg_mmap;     // user-virt Contiguous 8GB space including LMM space
  Ull  lmm_phys;     // kern-phys
  Ull  lmm_vadr;     // not used
  Ull  lmm_mmap;     // user-virt Contiguous 4GB space for LMM space
  Ull  ddr_phys;     // kern-phys
  Ull  ddr_vadr;     // not used
  Ull  ddr_mmap;     // user-virt Contiguous 4GB space in DDR-high-6GB space
  int  driver_use_1;
  int  driver_use_2;
} emax_info[EMAX_NLANE];

/*  ... for ARMSIML only */
#define DMA_BASE2_PHYS	 0x50000000
#define REG_BASE2_PHYS	 0x50100000
#define REG_CONF_OFFS    0x00002000
#define REG_BREG_OFFS    0x00004000
#define REG_ADDR_OFFS    0x00006000
#define REG_LDDM_OFFS    0x00008000
#define REG_AREA_MASK    0x0000ffff
#define LMM_BASE2_PHYS 	 0x60000000
#define MEM_VALID_ADDR	 0xafffffff

#ifndef NO_EMAX7LIB_BODY

#ifdef ARMZYNQ
/*******************************************************************************/
/******************************** ZYNQ-COMMON **********************************/
/*******************************************************************************/

#define DMA_BASE_PHYS	 0x00000000a4000000LL
#define DMA_BASE_PHYSOFS 0x0000000000010000LL
#define DMA_MMAP_SIZE	 0x0000000000010000LL
/*  ... 64KB  */
#define REG_BASE_PHYS	 0x0000020800000000LL
#define REG_BASE_PHYSOFS 0x0000000800000000LL
#define REG_MMAP_SIZE	 0x0000000200000000LL
/*  ... 8GB REGS(4G)+LMM(4G) */
#define LMM_BASE_PHYS	 0x0000020900000000LL
#define LMM_BASE_PHYSOFS 0x0000000800000000LL
/*  ... 4GB   */
#define DDR_BASE_PHYS	 0x0000050000000000LL
#define DDR_MMAP_SIZE	 0x0000000100000000LL
/*  ... 4GB   */

#define EMAX_IOC_MAGIC  60
/* Please use a different 8-bit number in your code */
#define EMAX_IORESET			_IO( EMAX_IOC_MAGIC, 0)
#define EMAX_GET_ACPMEM			_IOWR(EMAX_IOC_MAGIC,  1, unsigned long)
#define EMAX_IOC_MAXNR 2

static int filter(struct dirent *dir)
{
  return dir->d_name[0] == '.' ? 0 : 1;
}

static void trim(char *d_name)
{
  char *p = strchr(d_name, '\n');
  if (p != NULL) *p = '\0';
}

static int is_target_dev(char *d_name, char *target)
{
  char path[32];
  char name[32];
  FILE *fp;
  sprintf(path, "/sys/class/uio/%s/name", d_name);
  if ((fp = fopen(path, "r")) == NULL) return 0;
  if (fgets(name, sizeof(name), fp) == NULL) {
    fclose(fp);
    return 0;
  }
  fclose(fp);
  if (strcmp(name, target) != 0) return 0;
  return 1;
}

static int get_reg_size(char *d_name)
{
  char path[32];
  char size[32];
  FILE *fp;
  sprintf(path, "/sys/class/uio/%s/maps/map0/size", d_name);
  if ((fp = fopen(path, "r")) == NULL) return 0;
  if (fgets(size, sizeof(size), fp) == NULL) {
    fclose(fp);
    return 0;
  }
  fclose(fp);
  return strtoull(size, NULL, 16);
}

#ifdef __cplusplus
extern "C" {
#endif
int emax7_open(int NLANE);
#ifdef __cplusplus
}
#endif
#endif

/*******************************************************************************/
/******************************** Timer ****************************************/
/*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
void sleep_nanosec(int nano);
void reset_nanosec(int LANE);
void get_nanosec(int LANE, int class_no);
void show_nanosec(int LANE);
#ifdef __cplusplus
}
#endif

/*******************************************************************************/
/******************************** DMA-START ************************************/
/*******************************************************************************/

#if !defined(EMAXNC)
#ifdef __cplusplus
extern "C" {
#endif
int emax7_kick_dma(int LANE, int j);
void emax7_check_lmmi_and_dma(int LANE, int mode, int phase, int lastdist, int c, int i, int j);
void emax7_sigwait(int LANE);
#ifdef __cplusplus
}
#endif

#if defined(FPDDMA)
#define FPDDMA_DEFINED 1
static inline Ull arm64_read_dcache_line_size(void)
{
    Ull       ctr;
    Ull       dcache_line_size;
    const Ull bytes_per_word = 4;
    asm volatile ("mrs %0, ctr_el0" : "=r"(ctr) : : );
    asm volatile ("nop" : : : );
    dcache_line_size = (ctr >> 16) & 0xF;
    return (bytes_per_word << dcache_line_size);
}
static inline void arm64_flush_dcache_area(void* start, size_t size)
{
    Ull   vaddr           = (Ull)start;
    Ull   __end           = (Ull)start + size;
    Ull   cache_line_size = arm64_read_dcache_line_size();
    Ull   cache_line_mask = cache_line_size - 1;
    vaddr &= ~cache_line_mask;
    while (vaddr < __end) {
        asm volatile ("dc cvac, %0"  :  : "r"(vaddr) : );
        vaddr += cache_line_size;
    }
    asm volatile ("dsb	sy"  :  :  : );
}
static inline void arm64_flush_inv_dcache_area(void* start, size_t size)
{
    Ull   vaddr           = (Ull)start;
    Ull   __end           = (Ull)start + size;
    Ull   cache_line_size = arm64_read_dcache_line_size();
    Ull   cache_line_mask = cache_line_size - 1;
    vaddr &= ~cache_line_mask;
    while (vaddr < __end) {
        asm volatile ("dc civac, %0"  :  : "r"(vaddr) : );
        vaddr += cache_line_size;
    }
    asm volatile ("dsb	sy"  :  :  : );
}
#else
#define FPDDMA_DEFINED 0
#endif

#ifdef __cplusplus
extern "C" {
#endif
int emax7_kick_dma(int LANE, int j);
#ifdef __cplusplus
}
#endif

/*******************************************************************************/
/******************************** EMAX7-START **********************************/
/*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
void emax7_pre_with_keep_cache();
void emax7_pre_with_drain_cache();
#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************/
/******************************** EMAX7 NCLIB (no conv-c2c)*********************/
/*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
void cex(Uint op_cx, Ull *ex, Ull c3, Ull c2, Ull c1, Ull c0, Ushort pattern);
int convf32tou7(Uchar *out, float in);
int convf32tou8(Uchar *out, float in);
int convu8tof32(float *out, Uchar in);
Ull urand(int no);
Ull shfl(Ull in, Ull r);
void x11_softu64_dist(float, float);
int softu64(int stage, Ull *o1, Ull *o2, Ull *o3, Ull r1, Ull r2, Ull r3, Ull r4);
Ull exm(Ull s, Uchar exp);
int exe(Uint op_ex1, Ull *d, Ull s1, Uint exp1, Ull s2, Uint exp2, Ull s3, Uint exp3, Uint op_ex2, Ull r4, Uint op_ex3, Ull r5);
void mex(Uint op_mex2, Uchar **d2, Uchar *base2, Ull ofs2, Uint op_mex1, Uchar **d1, Uchar *base1, Ull ofs1, Ull limit, Ull s2, Ull s1);
void eag(Ull *adr, Ull base, Ull ofs);
void mop(Uint op_mm, Ull ex, Ull *d, Ull base, Ull offset, Uchar msk, Ull top, Uint len, Uint blk, Uchar force, Ull ptop, Uint plen);
void mo4(Uint op_mm, Ull ex, Ull *d, Ull base, Ull offset, Uchar msk, Ull top, Uint len, Uint blk, Uchar force, Ull ptop, Uint plen);
void mmp(Uint op_mm, Ull ex, Ull *d, Ull adr, Ull top, Uint len, Uint blk);
#ifdef __cplusplus
}
#endif
#endif
