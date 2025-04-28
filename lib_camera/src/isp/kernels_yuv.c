// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdlib.h>

#include "kernels_yuv.h"

#ifndef ALIGNED_4
#define ALIGNED_4 __attribute__((aligned(4)))
#endif


// kernels (original)
/*
static const int8_t a = 38, b = 75, c = 14;
static const int8_t d = -21, e = -42, f = 64;
static const int8_t g = 64, h = -53, i = -10;
*/

// kernels (wb)
static const int8_t a = 47, b = 61, c = 18;
static const int8_t d = -27, e = -34, f = 82;
static const int8_t g = 80, h = -43, i = -13;

// offsets (wb)
static const int8_t yk = 0;
static const int8_t uk = 21;
static const int8_t vk = 23;

// yuv0
const int8_t kernels_yuv_y0[32] ALIGNED_4 = {
    a, b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_u0[32] ALIGNED_4 = {
    d, e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_v0[32] ALIGNED_4 = {
    g, h, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_y1[32] ALIGNED_4 = {
    0, 0, a, b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// yuv1
const int8_t kernels_yuv_y2[32] ALIGNED_4 = {
    0, 0, 0, 0, a, b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_u2[32] ALIGNED_4 = {
    0, 0, 0, 0, d, e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_v2[32] ALIGNED_4 = {
    0, 0, 0, 0, g, h, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};
const int8_t kernels_yuv_y3[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, a, b, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0, 0, 0, 0, 
};

// yuv2
const int8_t kernels_yuv_y4[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, a, b, 0, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_u4[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, d, e, 0, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, f, 0, 0, 0, 0, 0, 0,
};
const int8_t kernels_yuv_v4[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, g, h, 0, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, i, 0, 0, 0, 0, 0, 0, 
};
const int8_t kernels_yuv_y5[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, a, b, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, c, 0, 0, 0, 0,  
};

// yuv3
const int8_t kernels_yuv_y6[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, a, b, 0, 0,   
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, c, 0, 0,
};
const int8_t kernels_yuv_u6[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, d, e, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, f, 0, 0,
};
const int8_t kernels_yuv_v6[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, g, h, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, i, 0, 0,
};
const int8_t kernels_yuv_y7[32] ALIGNED_4 = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, a, b,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, c,
};


//  Externs
const int8_t *yuv_kernels[16] = {
    
    kernels_yuv_v6, 
    kernels_yuv_y7,
    kernels_yuv_u6,
    kernels_yuv_y6,

    kernels_yuv_v4, 
    kernels_yuv_y5,
    kernels_yuv_u4, 
    kernels_yuv_y4, 
    
    kernels_yuv_v2, 
    kernels_yuv_y3,
    kernels_yuv_u2, 
    kernels_yuv_y2, 
    
    kernels_yuv_v0,
    kernels_yuv_y1, 
    kernels_yuv_u0, 
    kernels_yuv_y0,
};

const int16_t yuv_vsats[16] ALIGNED_4 = {
    7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7
};

const int8_t yuv_vadds[32] ALIGNED_4 = {
    yk, uk, yk, vk,
    yk, uk, yk, vk,
    
    yk, uk, yk, vk,
    yk, uk, yk, vk,

    yk, uk, yk, vk,
    yk, uk, yk, vk,

    yk, uk, yk, vk,
    yk, uk, yk, vk,
};
