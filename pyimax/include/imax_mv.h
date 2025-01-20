#pragma once
#include <stdio.h>
#include <math.h>

#if __cplusplus
extern "C" {
#endif
void imax_mv(unsigned char* values, unsigned char* keys, unsigned char* query, size_t qty, size_t size, size_t threadId);
#if __cplusplus
}
#endif