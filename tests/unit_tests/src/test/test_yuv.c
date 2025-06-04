// Copyright 2020-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <xcore/hwtimer.h>

#include "unity_fixture.h"

#include "kernels_yuv.h"
#include "camera_isp.h"
#include "helpers.h"

// Unity
TEST_GROUP_RUNNER(yuv) {
    RUN_TEST_CASE(yuv, yuv__simple);
}
TEST_GROUP(yuv);
TEST_SETUP(yuv) { fflush(stdout); print_separator("yuv");}
TEST_TEAR_DOWN(yuv) {}

extern void raw8_block_to_yuv422_prepare();
extern void raw8_block_to_yuv422(
    int8_t *dst,
    int8_t *src,
    unsigned line_size
);

extern void raw8_block_to_yuv422_inline(
    int8_t *dst,
    int8_t *src,
    unsigned line_size
);


static
void block_raw8_to_yuv422_vpu(
    int8_t *out_ptr, int8_t input_rows[2][MODE_YUV2_MAX_SIZE], 
    unsigned img_width){
    
    const unsigned steps = 16;
    unsigned loop_size = ((img_width << 1) - 4);
    const unsigned line_size = MODE_YUV2_MAX_SIZE;
    raw8_block_to_yuv422_prepare();
    for (unsigned x = 0; x <= (loop_size); x += steps) {
        int8_t *src = (int8_t *)&input_rows[0][x];
        int8_t *dst = &out_ptr[x];
        raw8_block_to_yuv422(dst, src, line_size);
    }
}

static
void block_raw8_to_yuv422_c(
    int8_t *out_ptr, int8_t input_rows[2][MODE_YUV2_MAX_SIZE], 
    unsigned img_width){
    
    const unsigned steps = 16;
    unsigned loop_size = ((img_width << 1) - 4);
    const unsigned line_size = MODE_YUV2_MAX_SIZE;
    raw8_block_to_yuv422_prepare();
    for (unsigned x = 0; x <= (loop_size); x += steps) {
        int8_t *src = (int8_t *)&input_rows[0][x];
        int8_t *dst = &out_ptr[x];
        raw8_block_to_yuv422_inline(dst, src, line_size);
    }
}


TEST(yuv, yuv__simple)
{
    const unsigned img_width = 64; // Must be >= 18 due to loop size = (img_width << 1) - 4
    int8_t input_rows[2][MODE_YUV2_MAX_SIZE];
    int8_t output[MODE_YUV2_MAX_SIZE * 2]; // Make sure it's large enough

    // Fill input 
    fill_array_rand_int8(input_rows[0], MODE_YUV2_MAX_SIZE);
    fill_array_rand_int8(input_rows[1], MODE_YUV2_MAX_SIZE);

    unsigned t0 = get_reference_time();
    // unsigned n_times = 10; // Number of times to run the function

    block_raw8_to_yuv422_vpu(output, input_rows, img_width);
    
    // block_raw8_to_yuv422_new(output, input_rows, img_width);
    unsigned t1 = get_reference_time();
    printf("Time taken: %u ticks\n", t1 - t0);

    // print X values
    unsigned x = 24;
    for (unsigned i = 0; i < x ; i++) {
        printf("%d ", output[i]);
    }

    block_raw8_to_yuv422_c(output, input_rows, img_width);
    // print Y values
    printf("\n");
    for (unsigned i = 0; i < x ; i++) {
        printf("%d ", output[i]);
    }
}
