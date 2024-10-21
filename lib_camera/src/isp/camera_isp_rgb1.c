#include <stdint.h>
#include <stdio.h>

#include <debug_print.h>

#include "camera.h"
#include "camera_isp.h"
#include "camera_utils.h"

#include "kernels_rgb1.h"

#define IMG_PTR(img, h, w, ch, width, channels) \
    ((img) + ((h) * (width) + (w)) * (channels) + (ch))


// ---------------------- VPU ASM ----------------------
// TODO if in the future is reused, move to a common file
inline void vpu_prepare_8(){
    asm volatile("vclrdr");
    asm volatile("ldc r11, 0x200");
    asm volatile("vsetc r11");
}
inline void vldc(const int8_t* ptr){
    asm volatile("vldc %0[0]" :: "r" (ptr));
}
inline void vlmaccr(const int8_t* ptr){
    asm volatile("vlmaccr %0[0]" :: "r" (ptr));
}
inline void vlsat16(const int16_t* shift){
    asm volatile("vlsat %0[0]" :: "r" (shift));
}
inline void vstrpv(int8_t* ptr, unsigned mask){
    asm volatile("vstrpv %0[0], %1" :: "r" (ptr), "r" (mask));
}
inline void vpu_vclrdr(){
    asm volatile("vclrdr");
}
inline void load_doubleword(int64_t *dst_ptr, int8_t *src_ptr) {
    uint32_t res[2];
    asm volatile("ldd %0, %1, %2[%3]" : "=r"(res[1]), "=r"(res[0]) : "r"(src_ptr), "r"(0));
    *dst_ptr = *((int64_t*)res);
}
inline void load_block(int8_t dst[32], int8_t* src, unsigned incr) {
    uint32_t *res = (uint32_t*)dst;
    asm volatile("ldd %0, %1, %2[0]" : "=r"(res[1]), "=r"(res[0]) : "r"(src)); src += incr;
    asm volatile("ldd %0, %1, %2[0]" : "=r"(res[3]), "=r"(res[2]) : "r"(src)); src += incr;
    asm volatile("ldd %0, %1, %2[0]" : "=r"(res[5]), "=r"(res[4]) : "r"(src)); src += incr;
    asm volatile("ldd %0, %1, %2[0]" : "=r"(res[7]), "=r"(res[6]) : "r"(src)); src += incr;
}
// ---------------------- Demosaic ----------------------

/**
 * @brief Demosaics a 4x8x1 RAW block into a 2x24x3 RGB block without downsampling.
 * 
 * This function processes a 4x8x1 RAW input block, reshaped as 1x32, and outputs
 * two rows of RGB data. (4x8x1 RAW => 2x24x3 RGB)    
 * 
 * The process involves splitting the input into 4 blocks of 
 * 12 operations each (total 48 ops). While it could be done in 3 blocks of 16 ops 
 * (VPU max), that approach would mix data from different rows.
 *
 * Output data is in RGB format, but the order can be modified by changing the kernel
 * pointer arrangement.
 * 
 * @param output1 Pointer to the first output row (RGB format).
 * @param output2 Pointer to the second output row (RGB format).
 * @param block_ptr Pointer to the input RAW block [32].
 */
static
void demosaic(
    int8_t *output1,
    int8_t *output2,
    int8_t block_ptr[32])
{
    vpu_prepare_8();
    vldc(block_ptr); // load pixels

    const unsigned str_arr[4] = {0,  12, 24, 36};
    const unsigned end_arr[4] = {12, 24, 36, 48};
    int8_t *outputs[4] = {&output1[0], &output1[12], &output2[0], &output2[12]};

    for (unsigned i = 0; i < 4; i++) {
        for (unsigned idx = str_arr[i]; idx < end_arr[i]; idx++) {
            vlmaccr(kernels_demosaic_12[idx]); // apply demosiac kernel
        }
        vlsat16(kernel_shifts); // right shift by 2
        vstrpv(outputs[i], KERNEL_MASK); // store pixels (24 pixels each row)
        vpu_vclrdr();
    }
}

void camera_isp_raw8_to_rgb1(image_cfg_t* image, int8_t* data_in, unsigned sensor_ln) {
    unsigned x1 = image->config->x1; //TODO create a logic to check only if needed
    unsigned y1 = image->config->y1;
    unsigned img_width = image->width;
    unsigned img_channels = image->channels;
    unsigned img_ln = sensor_ln - y1;
    unsigned row_offset = img_width * img_channels;
    int8_t *img_ptr = image->ptr;
    int8_t* data_src = data_in + x1;

    // 4 rows of 200 pixels
    static int8_t input_rows[4][MODE_RGB1_MAX_SIZE] ALIGNED_8 = { {0} };

    // if even, move data, if odd compute
    unsigned ln_is_even = (sensor_ln % 2 == 0);
    if (ln_is_even) {
        xmemcpy(&input_rows[0][0], &input_rows[2][0], img_width);     // move [2][x] to [0][x]
        xmemcpy(&input_rows[1][0], &input_rows[3][0], img_width);     // move [3][x] to [1][x]
        xmemcpy(&input_rows[2][0], data_src, img_width);              // move new data to [2][x]
    }
    else{ // if odd
        xmemcpy(&input_rows[3][0], data_src, img_width);              // move new data to [3][x]
        int8_t block[32] = {0};
        // for loop 8 by 8
        for (unsigned xpos = 0; xpos <= (img_width - 8); xpos += 8) {
            // construct the blocks 4x8 = 32 (to further fill vpu)
            load_block(&block[0], &input_rows[0][xpos], MODE_RGB1_MAX_SIZE);
            // demosaic, we assume red start
            int8_t *output1 = IMG_PTR(img_ptr, img_ln - 1, xpos, 0, img_width, img_channels);
            int8_t *output2 = output1 + row_offset; // Just move down one row
            demosaic(output1, output2, block);
        }
    }
}
