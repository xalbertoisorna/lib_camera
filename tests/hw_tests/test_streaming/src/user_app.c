// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcore/channel.h>
#include <xcore/hwtimer.h>
#include <xscope.h>

#include "camera.h"
#include "camera_utils.h"
#include "camera_isp.h"
#include "camera_io.h"

#define N_FRAMES    60
#define H           200
#define W           200

_Static_assert(CONFIG_STREAMING == 1, "camera_conf.h override not applied");

// ---- line callback: check rows arrive complete and in order ----
static volatile unsigned rows_in_frame = 0;
static volatile unsigned next_row = 0;
static volatile unsigned row_errors = 0;
static volatile unsigned restarts = 0;   // frame dropped by the ISP after some rows were delivered
static volatile uint32_t t_first_row = 0, t_last_row = 0;

void camera_isp_line_callback(const image_cfg_t* image, unsigned row, const int8_t* row_data) {
    (void)row_data;
    uint32_t t = get_reference_time();
    if (row == 0 && next_row != 0) {     // partial frame dropped, rows start again
        restarts++;
        rows_in_frame = 0;
        next_row = 0;
    }
    if (row != next_row) row_errors++;
    if (row == 0) t_first_row = t;
    if (row == image->height - 1) t_last_row = t;
    next_row = row + 1;
    rows_in_frame++;
}

static void reset_frame_stats(void) {
    rows_in_frame = 0;
    next_row = 0;
}

static void run_mode(chanend_t c_cam, image_cfg_t* image, const char* name, const char* file) {
    uint32_t t_done[N_FRAMES];
    unsigned bad_frames = 0;
    uint32_t rows_span_max = 0;

    camera_isp_coordinates_compute(image);
    camera_isp_prepare_capture(c_cam, image);   // let AE settle

    row_errors = 0;
    restarts = 0;
    uint32_t t_start = get_reference_time();
    for (unsigned i = 0; i < N_FRAMES; i++) {
        reset_frame_stats();
        camera_isp_start_capture(c_cam, image);
        camera_isp_get_capture(c_cam);
        t_done[i] = get_reference_time();
        if (rows_in_frame != image->height || next_row != image->height) bad_frames++;
        uint32_t span = t_last_row - t_first_row;
        if (span > rows_span_max) rows_span_max = span;
    }

    // steady state: skip the first frame (it waits for the next frame start)
    uint32_t dt_min = 0xFFFFFFFF, dt_max = 0;
    for (unsigned i = 2; i < N_FRAMES; i++) {
        uint32_t dt = t_done[i] - t_done[i - 1];
        if (dt < dt_min) dt_min = dt;
        if (dt > dt_max) dt_max = dt;
    }
    uint32_t total = t_done[N_FRAMES - 1] - t_done[1];
    float frame_ms = (total * 1e-5f) / (N_FRAMES - 2);
    printf("%s: %.2f ms/frame (%.2f fps), min %.2f max %.2f ms | first frame %.2f ms | "
           "rows per frame ok: %s (bad frames %u, row order errors %u, dropped+retried %u) | row0->last row %.2f ms\n",
           name, frame_ms, 1000.0f / frame_ms, dt_min * 1e-5f, dt_max * 1e-5f,
           (t_done[0] - t_start) * 1e-5f, (bad_frames || row_errors) ? "NO" : "yes", bad_frames, row_errors, restarts,
           rows_span_max * 1e-5f);

    camera_isp_stop(c_cam);   // stop streaming before writing to the host
    camera_io_write_image_file((char*)file, (uint8_t*)image->ptr, image->height, image->width, image->channels);
}

void user_app(chanend_t c_cam) {
    xscope_mode_lossless();

    static int8_t image_buffer[H * W * 3];
    camera_cfg_t config = { .offset_x = 0, .offset_y = 0, .mode = MODE_RGB2 };
    image_cfg_t image = {
        .height = H, .width = W, .channels = 3, .size = H * W * 3,
        .ptr = &image_buffer[0], .config = &config
    };

    config.mode = MODE_RGB1; run_mode(c_cam, &image, "RGB1", "stream_rgb1.rgb");
    config.mode = MODE_RGB2; run_mode(c_cam, &image, "RGB2", "stream_rgb2.rgb");
    config.mode = MODE_RGB4; run_mode(c_cam, &image, "RGB4", "stream_rgb4.rgb");

    // YUV422 (UVC friendly), 2 channels
    config.mode = MODE_YUV2;
    image.channels = 2; image.size = H * W * 2;
    run_mode(c_cam, &image, "YUV2", "stream_yuv2.yuv");

    camera_isp_stop(c_cam);   // sensor already stopped: returns immediately
    puts("done");
    exit(0);
}
