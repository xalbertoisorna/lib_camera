// Copyright 2020-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "unity_fixture.h"

#include "kernels_yuv.h"
#include "camera_isp.h"
#include "helpers.h"

#define WB_gain_RED    1.538
#define WB_gain_GREEN  1.0
#define WB_gain_BLUE   1.587
#define DELTA 5

// Unity
TEST_GROUP_RUNNER(yuv) {
    RUN_TEST_CASE(yuv, yuv__simple);
}
TEST_GROUP(yuv);
TEST_SETUP(yuv) { fflush(stdout); print_separator("yuv");}
TEST_TEAR_DOWN(yuv) {}


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
}
static inline void vpu_prepare_8(){
    asm volatile("ldc r11, 0x200");
    asm volatile("vsetc r11");
}
static inline void vpu_prepare_16(){
    asm volatile("ldc r11, 0x100");
    asm volatile("vsetc r11");
}
static inline void vldc(const int8_t* ptr){
    asm volatile("vldc %0[0]" :: "r" (ptr));
}
static inline void vlmaccr(const int8_t* ptr){
    asm volatile("vlmaccr %0[0]" :: "r" (ptr));
}

static inline void vlsat16(const int16_t* shift){
    asm volatile("vlsat %0[0]" :: "r" (shift));
}
static inline void vstr(int8_t* ptr){
    asm volatile("vstr %0[0]" :: "r" (ptr));
}
inline void vladd_16(int16_t* ptr){
    asm volatile("vladd %0[0]" :: "r" (ptr));
}


#define CLAMP(x) ((x < INT8_MIN) ? INT8_MIN : (x > INT8_MAX) ? INT8_MAX : x)

#define YC (30)
#define UC (24)
#define VC (27)

const int16_t aditions[32] ALIGNED_4 = {

};

const int16_t yuv_vsat2[16] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

const int8_t adds[16] = {
    YC, UC, YC, VC,
    YC, UC, YC, VC,
    YC, UC, YC, VC,
    YC, UC, YC, VC
};

// Tests
TEST(yuv, yuv__simple) // ensure we dont write zeros after the img
{
    const unsigned height = 2;
    const unsigned width = 24;
    const unsigned channels = 1;

    const int8_t a = 99, b = 127, c = 37;
    const int8_t d = -57, e = -74, f = 127;
    const int8_t g = 127, h = -94, i = -27;

    const int ac_rem = 7644; //a*84 + c*84;
    const int df_rem = 6216; //d*84 + f*84;
    const int gi_rem = 6956; //g*84 + i*84;

    const unsigned sat = 8;
    
    const unsigned size = height * width * channels;
    int8_t buff[size] ALIGNED_4 = { 0 };

    image_cfg_t image = {
		.height = height,
		.width = width,
		.channels = channels,
		.size = size,
		.ptr = buff,
		.config = NULL
	};
    fill_array_rand_int8(buff, size);
    print_array(buff, size);

    int8_t buff_vpu[32];
    int8_t res[32] = { 0 };
    int res16[32] = { 0 };
  
    block_2x16_vpu(buff_vpu, buff, width);
    print_array(buff_vpu, 32);

    /// --------------------- VPU ---------------------
    asm volatile("vclrdr");
    vpu_prepare_8();
    vldc(buff_vpu);
    for (unsigned i = 0 ; i < 16; i++)
    {
        vlmaccr(kernels_group[i]);
    }
    vlsat16(yuv_vsat);
    vstr(res);

    // remainders
    //vpu_prepare_16();
    //vladd_16(yuv_vadd);
    //vpu_prepare_8();
    
    // xor
    for (unsigned i = 0; i < 16; i++)
    {
        res16[i] = CLAMP(res[i] + adds[i]);
        res16[i] = ((int8_t)res16[i]);
        res16[i] = ((int8_t)res16[i]) ^ 0x80;
    }

    // ---------------------- VPU ---------------------

    int r0 = buff_vpu[0];
    int g0 = buff_vpu[1];
    int r1 = buff_vpu[2];
    int g1 = buff_vpu[3];
    int b0 = buff_vpu[17];
    int b1 = buff_vpu[19];

    // print pixel values
    printf("r0: %x\n", r0);
    printf("g0: %x\n", g0);
    printf("r1: %x\n", r1);
    printf("g1: %x\n", g1);
    printf("b0: %x\n", b0);
    printf("b1: %x\n", b1);
    printf("\n\n");


    int Y0 = (a * r0 + b * g0 + c * b0 + ac_rem) >> sat;
    int U0 = (d * r0 + e * g0 + f * b0 + df_rem) >> sat;
    int V0 = (g * r0 + h * g0 + i * b0 + gi_rem) >> sat;
    int Y1 = (a * r1 + b * g1 + c * b1 + ac_rem) >> sat;

    Y0 = CLAMP(Y0);
    U0 = CLAMP(U0);
    V0 = CLAMP(V0);
    Y1 = CLAMP(Y1);

    Y0 = ((int8_t)Y0) ^ 0x80;
    U0 = ((int8_t)U0) ^ 0x80;
    Y1 = ((int8_t)Y1) ^ 0x80;
    V0 = ((int8_t)V0) ^ 0x80;
    
    printf("Y0: %d\n", Y0);
    printf("U0: %d\n", U0);
    printf("Y1: %d\n", Y1);
    printf("V0: %d\n", V0);
    

    // print res
    printf("res: ");
    for (unsigned i = 0; i < 16; i++)
    {
        printf("%d ", res16[i]);
    }
    printf("\n");
}
