#include "../../include/imax_mv.h"
#include "../../include/imax_utils.h"
#include <stdio.h>

#define IMAX_KERNEL_COL_SIZE 56
#define NCHIP 1

void imax_mv(unsigned char* values, unsigned char* keys, unsigned char* query, size_t qty, size_t size, size_t threadId){
    int LANE = threadId;
    int imax_emb = qty;
    int imax_size = size;
    float *imax_key_array = (float*) keys;
    float *imax_query_array = (float*) query;
    float *imax_result_array = (float*) values;

    printf("imax_mv: qty=%d, size=%d, threadId=%d\n", qty, size, threadId);
    int imax_unit_col_size = imax_emb / IMAX_KERNEL_COL_SIZE;
    if ((imax_emb % IMAX_KERNEL_COL_SIZE) != 0) {
        imax_unit_col_size += 1;
    }

    reset_nanosec(threadId);
    for (int col_unit_blk_idx = 0; col_unit_blk_idx*imax_unit_col_size*IMAX_KERNEL_COL_SIZE < imax_emb; col_unit_blk_idx++) {
        Ull qaddr[IMAX_KERNEL_COL_SIZE];
        Ull kaddr[IMAX_KERNEL_COL_SIZE*4];
        Ull raddr[4];
        for (int j = 0; j < IMAX_KERNEL_COL_SIZE; j++) {
            for (int k = 0; k < 4; k++) {
                kaddr[(j*4) + k] = ((Ull)imax_key_array) + (((j*imax_unit_col_size)+col_unit_blk_idx)*imax_size)*4 + k*8;
            }
            qaddr[j] = ((Ull)imax_query_array) + ((j*imax_unit_col_size)+col_unit_blk_idx)*4;
            // printf("kaddr[%d]: %p, %p, %p, %p\n", j, kaddr[(j*4)+0], kaddr[(j*4)+1], kaddr[(j*4)+2], kaddr[(j*4)+3]);
            // printf("kaddr_val[%d]: %f, %f, %f, %f\n", j, *((float*)kaddr[(j*4)+0]), *((float*)kaddr[(j*4)+1]), *(((float*)kaddr[(j*4)+2])), *(((float*)kaddr[(j*4)+3])));
            // printf("qaddr[%d]: %p\n", j, qaddr[j]);
            // printf("kaddr[%d]: %p, \n", j, kaddr[(j*4)]);
        }
        for (int j = 0; j < 4; j++) {
            raddr[j] = ((Ull)imax_result_array) + j*8;
        }

        Ull CHIP, LOLP, INIT0, INIT1, LOOP0, LOOP1;
        Ull cofs, rofs, oofs, cofs1;
        Ull fetch_size = imax_size * imax_unit_col_size;
        Ull result_fetch_size = imax_size;
        Ull rofs_init = ((0-4*4LL)<<32)|((0-8*4LL)&0xFFFFFFFFLL);
        Ull cofs_init = ((0-4LL)<<32)|((0-(imax_size*4LL))&0xFFFFFFFFLL);
        Ull rofs_add = ((4*4LL)<<32)|((8*4LL)&0xFFFFFFFFLL);
        Ull cofs_add = (4LL<<32)|((imax_size*4LL)&0xFFFFFFFFLL);
        Ull BR[64][4][4], AR[64][4];

#define mv1_core(r, rm1, k0, k1, k2, k3, q) \
                    mop(OP_LDR,  1, &BR[rm1][0][1], (Ull)kaddr[k0], (Ull)cofs1, MSK_W0, (Ull)kaddr[k0], fetch_size, 0, 0, (Ull)NULL, fetch_size); \
                    mop(OP_LDR,  1, &BR[rm1][0][0], (Ull)kaddr[k1], (Ull)cofs1, MSK_W0, (Ull)kaddr[k0], fetch_size, 0, 0, (Ull)NULL, fetch_size); \
                    mop(OP_LDR,  1, &BR[rm1][1][1], (Ull)kaddr[k2], (Ull)cofs1, MSK_W0, (Ull)kaddr[k0], fetch_size, 0, 0, (Ull)NULL, fetch_size); \
                    mop(OP_LDR,  1, &BR[rm1][1][0], (Ull)kaddr[k3], (Ull)cofs1, MSK_W0, (Ull)kaddr[k0], fetch_size, 0, 0, (Ull)NULL, fetch_size); \
                    mop(OP_LDWR, 1, &BR[rm1][2][1], (Ull)qaddr[q],  (Ull)cofs,  MSK_W1, (Ull)qaddr[0],  imax_emb,   0, 0, (Ull)NULL, imax_emb  ); \
                    exe(OP_FMA, &AR[r][0], AR[rm1][0], EXP_H3210, BR[rm1][0][1], EXP_H3210, BR[rm1][2][1], EXP_H1010, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FMA, &AR[r][1], AR[rm1][1], EXP_H3210, BR[rm1][0][0], EXP_H3210, BR[rm1][2][1], EXP_H1010, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FMA, &AR[r][2], AR[rm1][2], EXP_H3210, BR[rm1][1][1], EXP_H3210, BR[rm1][2][1], EXP_H1010, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FMA, &AR[r][3], AR[rm1][3], EXP_H3210, BR[rm1][1][0], EXP_H3210, BR[rm1][2][1], EXP_H1010, OP_NOP, 0LL, OP_NOP, 0LL)

#define mv1_store(r, rm1) \
                    mop(OP_LDR, 3, &BR[r][0][1], (Ull)raddr[0], (Ull)oofs, MSK_W0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_LDR, 3, &BR[r][1][1], (Ull)raddr[1], (Ull)oofs, MSK_W0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_LDR, 3, &BR[r][2][1], (Ull)raddr[2], (Ull)oofs, MSK_W0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_LDR, 3, &BR[r][3][1], (Ull)raddr[3], (Ull)oofs, MSK_W0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    exe(OP_FAD, &AR[r][0], AR[rm1][0], EXP_H3210, BR[r][0][1], EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FAD, &AR[r][1], AR[rm1][1], EXP_H3210, BR[r][1][1], EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FAD, &AR[r][2], AR[rm1][2], EXP_H3210, BR[r][2][1], EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL); \
                    exe(OP_FAD, &AR[r][3], AR[rm1][3], EXP_H3210, BR[r][3][1], EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL); \
                    mop(OP_STR, 3, &AR[r][0], (Ull)oofs, (Ull)raddr[0], MSK_D0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_STR, 3, &AR[r][1], (Ull)oofs, (Ull)raddr[1], MSK_D0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_STR, 3, &AR[r][2], (Ull)oofs, (Ull)raddr[2], MSK_D0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size); \
                    mop(OP_STR, 3, &AR[r][3], (Ull)oofs, (Ull)raddr[3], MSK_D0, (Ull)raddr[0], result_fetch_size, 0, 1, (Ull)NULL, result_fetch_size)

//EMAX5A begin mv1 mapdist=0
        for (CHIP=0;CHIP<NCHIP;CHIP++) {
            for (INIT1=1,LOOP1=imax_unit_col_size,cofs=cofs_init;LOOP1--;INIT1=0) {
                for (INIT0=1,LOOP0=imax_size/8,rofs=rofs_init;LOOP0--;INIT0=0) {
                    exe(OP_ADD, &rofs, INIT0?rofs:rofs, EXP_H3210, rofs_add, EXP_H3210, 0LL, EXP_H3210, OP_AND, 0xffffffffffffffffLL, OP_NOP, 0LL);
                    exe(OP_ADD, &cofs, cofs, EXP_H3210, INIT0?cofs_add:0, EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);
                    exe(OP_ADD, &oofs, rofs, EXP_H1010, 0LL, EXP_H3210, 0LL, EXP_H3210, OP_AND, 0xffffffffLL, OP_NOP, 0LL);
                    exe(OP_ADD, &cofs1, cofs, EXP_H3210, rofs, EXP_H3210, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);

                    mop(OP_LDR,  1, &BR[2][0][1], (Ull)kaddr[0], (Ull)cofs1, MSK_W0, (Ull)kaddr[0], fetch_size, 0, 0, (Ull)NULL, fetch_size);
                    mop(OP_LDR,  1, &BR[2][0][0], (Ull)kaddr[1], (Ull)cofs1, MSK_W0, (Ull)kaddr[0], fetch_size, 0, 0, (Ull)NULL, fetch_size);
                    mop(OP_LDR,  1, &BR[2][1][1], (Ull)kaddr[2], (Ull)cofs1, MSK_W0, (Ull)kaddr[0], fetch_size, 0, 0, (Ull)NULL, fetch_size);
                    mop(OP_LDR,  1, &BR[2][1][0], (Ull)kaddr[3], (Ull)cofs1, MSK_W0, (Ull)kaddr[0], fetch_size, 0, 0, (Ull)NULL, fetch_size);
                    mop(OP_LDWR, 1, &BR[2][2][1], (Ull)qaddr[0], (Ull)cofs,  MSK_W1, (Ull)qaddr[0], imax_emb  , 0, 0, (Ull)NULL, imax_emb  );

                    exe(OP_FML, &AR[3][0], BR[2][0][1], EXP_H3210, BR[2][2][1], EXP_H1010, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);
                    exe(OP_FML, &AR[3][1], BR[2][0][0], EXP_H3210, BR[2][2][1], EXP_H1010, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);
                    exe(OP_FML, &AR[3][2], BR[2][1][1], EXP_H3210, BR[2][2][1], EXP_H1010, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);
                    exe(OP_FML, &AR[3][3], BR[2][1][0], EXP_H3210, BR[2][2][1], EXP_H1010, 0LL, EXP_H3210, OP_NOP, 0LL, OP_NOP, 0LL);

                    mv1_core( 4,  3,  4,  5,  6,  7,  1);
                    mv1_core( 5,  4,  8,  9, 10, 11,  2);
                    mv1_core( 6,  5, 12, 13, 14, 15,  3);
                    mv1_core( 7,  6, 16, 17, 18, 19,  4);
                    mv1_core( 8,  7, 20, 21, 22, 23,  5);
                    mv1_core( 9,  8, 24, 25, 26, 27,  6);
                    mv1_core(10,  9, 28, 29, 30, 31,  7);
                    mv1_core(11, 10, 32, 33, 34, 35,  8);
                    mv1_core(12, 11, 36, 37, 38, 39,  9);
                    mv1_core(13, 12, 40, 41, 42, 43, 10);
                    mv1_core(14, 13, 44, 45, 46, 47, 11);
                    mv1_core(15, 14, 48, 49, 50, 51, 12);
                    mv1_core(16, 15, 52, 53, 54, 55, 13);
                    mv1_core(17, 16, 56, 57, 58, 59, 14);
                    mv1_core(18, 17, 60, 61, 62, 63, 15);
                    mv1_core(19, 18, 64, 65, 66, 67, 16);
                    mv1_core(20, 19, 68, 69, 70, 71, 17);
                    mv1_core(21, 20, 72, 73, 74, 75, 18);
                    mv1_core(22, 21, 76, 77, 78, 79, 19);
                    mv1_core(23, 22, 80, 81, 82, 83, 20);
                    mv1_core(24, 23, 84, 85, 86, 87, 21);
                    mv1_core(25, 24, 88, 89, 90, 91, 22);
                    mv1_core(26, 25, 92, 93, 94, 95, 23);
                    mv1_core(27, 26, 96, 97, 98, 99, 24);
                    mv1_core(28, 27,100,101,102,103, 25);
                    mv1_core(29, 28,104,105,106,107, 26);
                    mv1_core(30, 29,108,109,110,111, 27);
                    mv1_core(31, 30,112,113,114,115, 28);
                    mv1_core(32, 31,116,117,118,119, 29);
                    mv1_core(33, 32,120,121,122,123, 30);
                    mv1_core(34, 33,124,125,126,127, 31);
                    mv1_core(35, 34,128,129,130,131, 32);
                    mv1_core(36, 35,132,133,134,135, 33);
                    mv1_core(37, 36,136,137,138,139, 34);
                    mv1_core(38, 37,140,141,142,143, 35);
                    mv1_core(39, 38,144,145,146,147, 36);
                    mv1_core(40, 39,148,149,150,151, 37);
                    mv1_core(41, 40,152,153,154,155, 38);
                    mv1_core(42, 41,156,157,158,159, 39);
                    mv1_core(43, 42,160,161,162,163, 40);
                    mv1_core(44, 43,164,165,166,167, 41);
                    mv1_core(45, 44,168,169,170,171, 42);
                    mv1_core(46, 45,172,173,174,175, 43);
                    mv1_core(47, 46,176,177,178,179, 44);
                    mv1_core(48, 47,180,181,182,183, 45);
                    mv1_core(49, 48,184,185,186,187, 46);
                    mv1_core(50, 49,188,189,190,191, 47);
                    mv1_core(51, 50,192,193,194,195, 48);
                    mv1_core(52, 51,196,197,198,199, 49);
                    mv1_core(53, 52,200,201,202,203, 50);
                    mv1_core(54, 53,204,205,206,207, 51);
                    mv1_core(55, 54,208,209,210,211, 52);
                    mv1_core(56, 55,212,213,214,215, 53);
                    mv1_core(57, 56,216,217,218,219, 54);
                    mv1_core(58, 57,220,221,222,223, 55);


                    // mv1_core(58, 57,224,225,226,227, 56);
                    // mv1_core(59, 58,228,229,230,231, 57);
                    // mv1_core(60, 59,232,233,234,235, 58);
                    // mv1_core(61, 60,236,237,238,239, 59);

                    mv1_store(62, 58);
                }
            }
        }
//EMAX5A end
    }
//EMAX5A drain_dirty_lmm
    #ifndef ARMZYNQ
    get_nanosec(threadId, 0);
    #endif
    show_nanosec(threadId);

    // printf("IMAX Result: [");
    // for (int j = 0; j < size; j++) {
        // printf("%f", imax_result_array[j]);
        // if (j < size - 1) {
            // printf(", ");
        // }
    // }
    // printf("]\n");
    // printf("IMAX Done\n");
}