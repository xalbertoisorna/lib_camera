// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcore/channel.h>
#include <xcore/assert.h>

#include "camera.h"
#include "camera_utils.h"
#include "camera_isp.h"
#include "camera_io.h"

#define H   400
#define W   400
#define CH    1 // RAW

// camera_conf.h in this application overrides the library default
_Static_assert(CONFIG_FLIP == FLIP_VERTICAL, "camera_conf.h override not applied");

void user_app(chanend_t c_cam) {
    camera_cfg_t config = {
        // centred ROI: same scene region with and without flip
        .offset_x = 0.25,
        .offset_y = 0.25,
        .mode = MODE_RAW,
    };

    int8_t image_buffer[H][W][CH] = {{{0}}};
    image_cfg_t image = {
        .height = H,
        .width = W,
        .channels = CH,
        .size = H * W * CH,
        .ptr = &image_buffer[0][0][0],
        .config = &config
    };

    // let the sensor settle (AE/AWB)
    delay_seconds_cpp(3);

    camera_isp_coordinates_compute(&image);

    // with vertical flip the ROI is shifted one line to keep the Bayer pattern
    unsigned y1_expected = ((unsigned)(config.offset_y * SENSOR_HEIGHT) & ~1) + 1;
    printf("Flip vertical: y1=%u (expected %u)\n", config.y1, y1_expected);
    xassert(config.y1 == y1_expected && "ROI not shifted for vertical flip");

    camera_isp_start_capture(c_cam, &image);
    camera_isp_get_capture(c_cam);
    camera_io_write_image_file("capture_flip.raw", (uint8_t*)image.ptr, H, W, CH);
    puts("capture done");
    exit(0);
}
