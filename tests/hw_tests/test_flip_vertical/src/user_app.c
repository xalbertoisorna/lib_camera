// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcore/channel.h>
#include <xcore/assert.h>
#include <xcore/hwtimer.h>
#include <xscope.h>

#include "camera.h"
#include "camera_utils.h"
#include "camera_isp.h"
#include "camera_io.h"

#define FILE1_NAME "capture1.rgb"
#define FILE2_NAME "capture2.rgb"

// camera_conf.h in this application overrides the library default
_Static_assert(CONFIG_FLIP == FLIP_VERTICAL, "camera_conf.h override not applied");

static
void save_image(image_cfg_t* image, char* filename) {
    uint8_t * img_ptr = (uint8_t*)image->ptr;
    camera_io_write_image_file(filename, img_ptr, image->height, image->width, image->channels);
}

static
void check_flip_offset(camera_cfg_t* config) {
    // with vertical flip the ROI is shifted one line to keep the Bayer pattern
    unsigned y1_expected = ((unsigned)(config->offset_y * SENSOR_HEIGHT) & ~1) + 1;
    printf("Flip vertical: y1=%u (expected %u)\n", config->y1, y1_expected);
    xassert(config->y1 == y1_expected && "ROI not shifted for vertical flip");
}

void user_app(chanend_t c_cam) {

    // lossless xscope so image file writes to the host are not truncated
    xscope_mode_lossless();

    // Image and configuration (same as examples/capture_rgb)
    const unsigned h = 200;
    const unsigned w = 200;
    const unsigned ch = 3;
    const unsigned img_size = h * w * ch;
    int8_t image_buffer[img_size] = { 0 };

    camera_cfg_t config = {
        .offset_x = 0,
        .offset_y = 0,
        .mode = MODE_RGB2,
    };
    image_cfg_t image = {
        .height = h,
        .width = w,
        .channels = ch,
        .size = h*w*ch,
        .ptr = &image_buffer[0],
        .config = &config
    };

    // first capture
    camera_isp_coordinates_compute(&image);
    check_flip_offset(&config);
    camera_isp_start_capture(c_cam, &image);
    camera_isp_get_capture(c_cam);
    save_image(&image, FILE1_NAME);

    // change coordinates and time the capture
    config.offset_x = 0.5;
    config.offset_y = 0.1;
    unsigned t0 = get_reference_time();
    camera_isp_coordinates_compute(&image);
    camera_isp_start_capture(c_cam, &image);
    camera_isp_get_capture(c_cam);
    unsigned t1 = get_reference_time();
    printf("Capture time:\t%f [ms]\n", (t1 - t0) * 1e-5);
    check_flip_offset(&config);
    save_image(&image, FILE2_NAME);

    puts("capture done");
    exit(0);
}
