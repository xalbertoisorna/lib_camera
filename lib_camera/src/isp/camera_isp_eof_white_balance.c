// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xcore/assert.h>

#include <print.h>
#include "camera_isp.h"

// ------ kernels ------
static const int8_t WB_KR = 45;
static const int8_t WB_KG = 0;
static const int8_t WB_KB = 47;

static const int8_t WB_CR = 12;
static const int8_t WB_CG = 8;
static const int8_t WB_CB = 13;

static const int8_t WB_SAT = 3;

static
const int8_t vk_rgb[32] ALIGNED_4 = {
    WB_KR, WB_KG, WB_KB, 
    WB_KR, WB_KG, WB_KB, 
    WB_KR, WB_KG, WB_KB,
    WB_KR, WB_KG, WB_KB, // 12
    0, 0, 0, 0, // 16
    0, 0, 0, 0, // 20
    0, 0, 0, 0, // 24
    0, 0, 0, 0, // 28
    0, 0, 0, 0, // 32
};

static
const int8_t vc_rgb[32] ALIGNED_4 = {
    WB_CR, WB_CG, WB_CB,
    WB_CR, WB_CG, WB_CB,
    WB_CR, WB_CG, WB_CB,
    WB_CR, WB_CG, WB_CB, // 12
    0, 0, 0, 0, // 16
    0, 0, 0, 0, // 20
    0, 0, 0, 0, // 24
    0, 0, 0, 0, // 28
    0, 0, 0, 0, // 32
};

static
const int16_t vsat[16] ALIGNED_4 = {
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT
};

// ------------ Defined in rgb_img_wb.S ------------------
extern void wb_scale_image(
    image_cfg_t* image,
    const int8_t vk[32], 
    const int8_t vc[32],
    const int16_t vsat[16]
);

// ----------------- High Level API ---------------------
void camera_isp_white_balance(image_cfg_t* image)
{
    xassert(image->ptr != NULL && "image pointer is NULL");
    xassert(image->size > 12 && "image size should be > 12 bytes");
    wb_scale_image(image, vk_rgb, vc_rgb, vsat);
}

void camera_isp_white_balance_rows(image_cfg_t* image, unsigned row, unsigned num_rows)
{
    xassert(image->ptr != NULL && "image pointer is NULL");
    xassert(row + num_rows <= image->height && "rows out of range");
    image_cfg_t rows = *image;
    rows.height = num_rows;
    rows.size = num_rows * image->width * image->channels;
    rows.ptr = image->ptr + (row * image->width * image->channels);
    wb_scale_image(&rows, vk_rgb, vc_rgb, vsat);
}
