#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <print.h>
#include "camera_isp.h"

#ifdef DEBUG_PRINT_ENABLE_CAM_ISP
#include <xcore/hwtimer.h>
#include <debug_print.h>
#endif

#ifndef ALIGNED_4
#define ALIGNED_4 __attribute__((aligned(4)))
#endif

// ------ kernels ------
#define WB_KR 44   
#define WB_KG 0
#define WB_KB 44
#define WB_CR 6
#define WB_CG 4
#define WB_CB 6
#define WB_SAT 2

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

const int16_t vsat[16] ALIGNED_4 = {
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT,
    WB_SAT,WB_SAT,WB_SAT,WB_SAT
};

// ------ ASM ------
extern void wb_scale(
    int8_t px[32],
    const int8_t vk[32], 
    const int8_t vc[32],
    const int16_t vsat[16]
);

extern void wb_scale_image(
    int8_t *img,
    unsigned img_size,
    const int8_t vk[32], 
    const int8_t vc[32],
    const int16_t vsat[16]
);

// ------ Static ------

/*
static
inline void vpu_prepare(){
    asm volatile("vclrdr");
    asm volatile("ldc r11, 0x200");
    asm volatile("vsetc r11");
}

static inline
void vpu_wb(int8_t* arr, unsigned arr_size){
    unsigned i = 0;

    // Prepare VPU for int8 mode
    vpu_prepare();

    // do WB aWB_CRoss all the image
    for (i = 0; i <= (arr_size - 32); i+=12)
    {
        wb_scale(&arr[i], vk_rgb, vc_rgb, vsat);
    }

    // treat the remaining pixels
    int remaining = arr_size - i;
    if (remaining){
        int8_t tmp[64] ALIGNED_4 = {0};
        memcpy(tmp, &arr[i], remaining);
        wb_scale(&tmp[0], vk_rgb, vc_rgb, vsat);
        wb_scale(&tmp[12], vk_rgb, vc_rgb, vsat);
        wb_scale(&tmp[24], vk_rgb, vc_rgb, vsat);
        memcpy(&arr[i], tmp, remaining);
    }
}
*/


static inline
void vpu_wb(int8_t* arr, unsigned arr_size){
    wb_scale_image(arr, arr_size, vk_rgb, vc_rgb, vsat);
}


// ------ Public ------
void camera_isp_swb(image_cfg_t* image)
{
    if (image->ptr == NULL) {
        return;
    }
    vpu_wb(image->ptr, image->size);
}
