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

extern void raw8_block_to_yuv422(
    int8_t *dst,
    int8_t *src,
    unsigned line_size
);

// VPU but not working properly version
static
void block_raw8_to_yuv422_new(
    int8_t *out_ptr, int8_t input_rows[2][MODE_YUV2_MAX_SIZE], 
    unsigned img_width){
    
    const unsigned steps = 16;
    const unsigned loop_size = ((img_width << 1) - 4);
    const unsigned line_size = MODE_YUV2_MAX_SIZE;

    for (unsigned x = 0; x <= (loop_size); x += steps) {
        
        // load block of 2x16 pixels
        int8_t *src = (int8_t *)&input_rows[0][x];
        int8_t *dst = &out_ptr[x];
        raw8_block_to_yuv422(dst, src, line_size);
    }
}


TEST(yuv, yuv__simple)
{
    const unsigned img_width = 64; // Must be >= 18 due to loop size = (img_width << 1) - 4
    int8_t input_rows[2][MODE_YUV2_MAX_SIZE] = {0};
    int8_t output[MODE_YUV2_MAX_SIZE * 2] = {0}; // Make sure it's large enough

    // Fill input with a pattern: values between -128 to 127
    for (unsigned i = 0; i < img_width * 2; i++) {
        input_rows[0][i] = (int8_t)(i - 64);         // [-64, ...]
        input_rows[1][i] = (int8_t)(127 - i);        // [63, ...]
    }

    unsigned t0 = get_reference_time();
    block_raw8_to_yuv422_new(output, input_rows, img_width);
    unsigned t1 = get_reference_time();
    printf("Time taken: %u ticks\n", t1 - t0);

    // print X values
    unsigned x = 24;
    for (unsigned i = 0; i < x ; i++) {
        printf("%d ", output[i]);
    }

    int expected[] = {92, -1, 94, 119, 95, -1, 96, 121, 98, -5, 99, 122, 101, -9, 102, 124, 103, -14, 105, 126, 106, -18, 108, 127};
    for (unsigned i = 0; i < x; i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], output[i]);
    }
}
