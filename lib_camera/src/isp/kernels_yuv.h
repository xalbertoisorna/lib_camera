// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdlib.h>

#ifndef ALIGNED_4
#define ALIGNED_4 __attribute__((aligned(4)))
#endif

// kernels
const int8_t a = 99, b = 127, c = 37;
const int8_t d = -57, e = -74, f = 127;
const int8_t g = 127, h = -94, i = -27;

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



// now remainders

const int8_t rem0[32] ALIGNED_4 = {
    77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t rem1[32] ALIGNED_4 = {
    62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t rem2[32] ALIGNED_4 = {
    70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const int8_t rem3[32] ALIGNED_4 = {
    78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


//  Externs

/*
const int8_t *kernels_group[16] = {
    kernels_yuv_y0, kernels_yuv_u0, kernels_yuv_v0, kernels_yuv_y1,
    kernels_yuv_y2, kernels_yuv_u2, kernels_yuv_v2, kernels_yuv_y3,
    kernels_yuv_y4, kernels_yuv_u4, kernels_yuv_v4, kernels_yuv_y5,
    kernels_yuv_y6, kernels_yuv_u6, kernels_yuv_v6, kernels_yuv_y7
};
*/

const int8_t *kernels_group[16] = {
    
    kernels_yuv_y7,
    kernels_yuv_v6, 
    kernels_yuv_u6,
    kernels_yuv_y6,

    kernels_yuv_y5,
    kernels_yuv_v4, 
    kernels_yuv_u4, 
    kernels_yuv_y4, 

    kernels_yuv_y3,
    kernels_yuv_v2, 
    kernels_yuv_u2, 
    kernels_yuv_y2, 
    
    kernels_yuv_y1,
    kernels_yuv_v0, 
    kernels_yuv_u0, 
    kernels_yuv_y0,
};


const int8_t *remainders_group[16] = {
    rem0, rem1, rem2, rem3,
    rem0, rem1, rem2, rem3,
    rem0, rem1, rem2, rem3,
    rem0, rem1, rem2, rem3
};

const int8_t vcrem[32] ALIGNED_4 = {
    100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, c,
};


const int16_t yuv_vsat[16] ALIGNED_4 = {
    8,8,8,8,
    8,8,8,8,
    8,8,8,8,
    8,8,8,8
};
