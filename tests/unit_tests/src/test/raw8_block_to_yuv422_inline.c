// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <debug_print.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>

#include "camera.h"
#include "camera_isp.h"
#include "camera_utils.h"

#include "kernels_yuv.h"

static inline
void vldr16(
    const uint32_t mask,
    uint32_t *in,
    uint32_t *out)
{
    register int32_t *ptr asm("r11") = (int32_t *)in;
    asm volatile("vldr %0[0]" :: "r" (ptr));
    asm volatile("vstrpv %0[0], %1" ::"r"(out), "r"(mask));
}
static inline 
void block_2x16_vpu(
    int8_t dst[32],
    int8_t *src,
    unsigned line_size)
{
    const uint32_t mask = (1 << 16) - 1;
    vldr16(mask, (uint32_t *)src, (uint32_t *)dst);
    vldr16(mask, (uint32_t *)(src + line_size), (uint32_t *)(dst + 16));
    asm volatile("vclrdr");
}
static inline void vpu_prepare_8(){
    asm volatile("vclrdr");
    asm volatile("ldc r11, 0x200");
    asm volatile("vsetc r11");
}
static inline void vldc(const int8_t* ptr){
    asm volatile("vldc %0[0]" :: "r" (ptr));
}
static inline void vlmaccr(const int8_t* ptr){
    asm volatile("vlmaccr %0[0]" :: "r" (ptr));
}
static inline void vstr(int8_t* ptr){
    asm volatile("vstr %0[0]" :: "r" (ptr));
}
static inline void vladd(const int8_t* ptr){
    asm volatile("vladd %0[0]" :: "r" (ptr));
}
static inline void vlsat(const int16_t* shift){
    asm volatile("vlsat %0[0]" :: "r" (shift));
}



///-----------------------------
///------------------------------
///-----------------------------
///------------------------------
///-----------------------------
///------------------------------
///-----------------------------
///------------------------------
///-----------------------------
///------------------------------

// kernels (with wb)
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


void raw8_block_to_yuv422_inline(
    int8_t *dst,
    int8_t *src,
    unsigned line_size
){
    int8_t vpu_vc[32] = {0};
    vpu_prepare_8();
    block_2x16_vpu(vpu_vc, src, line_size); // warning uses R11
    vldc(vpu_vc);

    // kernel multiplication
    #pragma clang loop unroll(full)
    for (unsigned i = 0; i < 16; i++)
    {
        vlmaccr(yuv_kernels[i]);
    }
    vlsat(yuv_vsats);
    vladd(yuv_vadds);
    vstr(dst);
}

/*
// slow C version, we can only afford half screen
#define CLAMP(x) ((x < INT8_MIN) ? INT8_MIN : (x > INT8_MAX) ? INT8_MAX : x)

static
void block_raw8_to_yuv422_c(
    int8_t *out_ptr, int8_t input_rows[2][MODE_YUV2_MAX_SIZE], 
    unsigned img_width){
    
    // YUV (BT.601) fixed point coeffs
    // const int Y_coeff[3] = {66, 129, 25};
    // const int U_coeff[3] = {-38, -74, 112};
    // const int V_coeff[3] = {112, -94, -18};

    // Corrected YUV
    const int a = 99, b = 129, c = 37;
    const int d = -57, e = -74, f = 168;
    const int g = 168, h = -94, i = -27;
    const int ac_rem = 7644; //a*84 + c*84;
    const int df_rem = 6216; //d*84 + f*84;
    const int gi_rem = 6956; //g*84 + i*84;
    const unsigned steps = 4;
    unsigned loop_size = ((img_width << 1) - 4);
    
    for (unsigned x = 0; x <= loop_size; x += steps) {
        // Load 2 RAW pixels
        int r0 = input_rows[0][x+0];
        int g0 = input_rows[0][x+1];
        int r1 = input_rows[0][x+2];
        int g1 = input_rows[0][x+3];
        int b0 = input_rows[1][x+1];
        int b1 = input_rows[1][x+3];

        // YUV conversion
        int Y0 = (a * r0 + b * g0 + c * b0 + ac_rem) >> 8;
        int U0 = (d * r0 + e * g0 + f * b0 + df_rem) >> 8;
        int V0 = (g * r0 + h * g0 + i * b0 + gi_rem) >> 8;
        int Y1 = (a * r1 + b * g1 + c * b1 + ac_rem) >> 8;

        // clamp everything int8 range
        Y0 = CLAMP(Y0);
        U0 = CLAMP(U0);
        V0 = CLAMP(V0);
        Y1 = CLAMP(Y1);

        // Output
        out_ptr[x+0] = ((int8_t)Y0) ^ 0x80;
        out_ptr[x+1] = ((int8_t)U0) ^ 0x80;
        out_ptr[x+2] = ((int8_t)Y1) ^ 0x80;
        out_ptr[x+3] = ((int8_t)V0) ^ 0x80;
    }
}
*/
