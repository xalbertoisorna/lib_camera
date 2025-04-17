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

#define CLAMP(x) ((x < INT8_MIN) ? INT8_MIN : (x > INT8_MAX) ? INT8_MAX : x)

static
void block_raw8_to_yuv422(int8_t *out_ptr, int8_t input_rows[2][MODE_YUV2_MAX_SIZE], unsigned img_width){
    
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
    loop_size = loop_size >> 1;
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

void camera_isp_raw8_to_yuv2(image_cfg_t* image, int8_t* data_in, unsigned sensor_ln){
    unsigned x1 = image->config->x1;
    unsigned y1 = image->config->y1;
    unsigned img_width = image->width;
    unsigned img_channels = image->channels;
    int8_t* data_src = data_in + x1;
    int8_t* img_ptr = image->ptr;

    // 2 rows of 400 pixels
    static int8_t input_rows[2][MODE_YUV2_MAX_SIZE] ALIGNED_8 = { {0} };
    unsigned buff_ln = sensor_ln % 2;
    xmemcpy(&input_rows[buff_ln][0], data_src, img_width * 2);
    if(buff_ln == 1) {
        unsigned img_ln = (sensor_ln - y1 - 1) >> 1;
        int8_t *out_ptr = img_ptr + ((img_ln * img_width)) * (img_channels);
        block_raw8_to_yuv422(out_ptr, input_rows, img_width);
    }
}
