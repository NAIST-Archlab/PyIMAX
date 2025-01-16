#pragma once
#ifndef IMAX_UTILS_H
#define IMAX_UTILS_H
#include <stdio.h>
#include <math.h>
#if __cplusplus
#ifndef UTYPEDEF
#define UTYPEDEF
typedef unsigned char      Uchar;
typedef unsigned short     Ushort;
typedef unsigned int       Uint;
typedef unsigned long long Ull;
typedef long long int      Sll;
#if __AARCH64EL__ == 1
typedef long double Dll;
#else
typedef struct {Ull u[2];} Dll;
#endif
#endif
#else
#include "conv-c2d/emax7lib.h"
#endif

#if __cplusplus
extern "C" {
#endif
void imemcpy(Uint *dst, Uint *src, int words);
void xmax_bzero(Uint *dst, int words);
unsigned char* sysinit(Uint memsize, Uint alignment, Uint threadQty);
#if __cplusplus
}
#endif
#endif