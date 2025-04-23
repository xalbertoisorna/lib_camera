// Copyright 2020-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "unity_fixture.h"

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
    asm("vldr %0[0]" :: "r" (ptr));
    asm("vstrpv %0[0], %1" ::"r"(out), "r"(mask));
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

static inline void vlsat16(const int16_t* shift){
    asm volatile("vlsat %0[0]" :: "r" (shift));
}
static inline void vstr(int8_t* ptr){
    asm volatile("vstr %0[0]" :: "r" (ptr));
}


#include "kernels_yuv.h"

// Tests
TEST(yuv, yuv__simple) // ensure we dont write zeros after the img
{
    const unsigned height = 2;
    const unsigned width = 24;
    const unsigned channels = 1;

    const int ac_rem = 7644; //a*84 + c*84;
    const int df_rem = 6216; //d*84 + f*84;
    const int gi_rem = 6956; //g*84 + i*84;


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
    block_2x16_vpu(buff_vpu, buff, width);
    print_array(buff_vpu, 32);


    vpu_prepare_8();
    vldc(buff_vpu);
    for (unsigned i = 0; i < 16; i++)
    {
        vlmaccr(kernels_group[i]);
    }
    vldc(vcrem);
    for (unsigned i = 0; i < 16; i++)
    {
        vlmaccr(remainders_group[i]);
    }
    vlsat16(yuv_vsat);
    

    int r0 = buff_vpu[0];
    int g0 = buff_vpu[1];
    int b0 = buff_vpu[16+1];
    int Y0 = (a * r0 + b * g0 + c * b0 + ac_rem) >> 8;

    printf("Y0: %d\n", Y0);
    assert(0);
}
