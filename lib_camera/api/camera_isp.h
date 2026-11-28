// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.



#pragma once

#include <stdint.h>

#include "api.h"
#include "camera.h" // packet size

C_API_START

/** 
 * @defgroup camera_isp_cfg Functions related to the Image Signal Processing (ISP) pipeline.
 * @{
 */


// Maximum buffer sizes for each camera mode.
// Note: Values are derived from the base SENSOR_WIDTH, accounting for 
// resolution scaling and hardware alignment/parity constraints.
// RAW, RGB4 and YUV2 hold full sensor rows, so they grow with SENSOR_WIDTH above 800.
#define MODE_MAX_SIZE_800_OR_SENSOR ((SENSOR_WIDTH) > 800 ? (SENSOR_WIDTH) : 800)
#define MODE_RAW_MAX_SIZE           MODE_MAX_SIZE_800_OR_SENSOR
#define MODE_RGB1_MAX_SIZE          (200)
#define MODE_RGB2_MAX_SIZE          (400)
#define MODE_RGB4_MAX_SIZE          MODE_MAX_SIZE_800_OR_SENSOR
#define MODE_YUV2_MAX_SIZE          MODE_MAX_SIZE_800_OR_SENSOR

// ---------------------------- ISP CFG -------------------------------

typedef unsigned mipi_header_t;

/// @brief Defines the camera output modes.
typedef enum {
  MODE_RAW  = 0,   ///< Unprocessed RAW sensor data
  MODE_RGB1 = 1,   ///< RGB mode with no downsample (demosaicing)
  MODE_RGB2 = 2,   ///< RGB mode with x2 downsample
  MODE_RGB4 = 4,   ///< RGB mode with x4 downsample
  MODE_YUV2 = 6,   ///< YUV2 mode (YUV422)
} camera_mode_t;

/// @brief Configuration structure for the camera.
typedef struct
{
  float offset_x;         ///< Horizontal offset in [0,1] range relative to the sensor area
  float offset_y;         ///< Vertical offset in [0,1] range relative to the sensor area
  camera_mode_t mode;     ///< Camera output mode
  unsigned x1, y1, x2, y2;///< Region of interest (ROI) in MIPI coordinates
  unsigned sensor_width;  ///< Width of the MIPI region
  unsigned sensor_height; ///< Height of the MIPI region
} camera_cfg_t;

/// @brief Image configuration structure.
typedef struct {
  unsigned height;    ///< Height of the output image in pixels
  unsigned width;     ///< Width of the output image in pixels
  unsigned channels;  ///< Number of channels in the output image (ex: 1 for RAW, 3 for RGB)
  unsigned size;      ///< Size of the output image in bytes
  int8_t* ptr;        ///< Pointer to the output image data
  camera_cfg_t* config; ///< Pointer to the camera configuration structure
  /**
   * Rows held by the output buffer (MODE_RAW and MODE_YUV2 only). 0: the buffer holds the whole image.
   * Otherwise the buffer is a ring of `ring_rows` rows: row `r` is written at `r % ring_rows`, so a large image
   * can be streamed through `camera_isp_line_callback` without storing it whole. The consumer must copy each row
   * before the ISP wraps round to it. Auto exposure is not computed from ring images.
   */
  unsigned ring_rows;
} image_cfg_t;

/**
 * @brief Pointer to output row `row` of an image, taking `ring_rows` into account.
 */
static inline int8_t* camera_isp_row_ptr(const image_cfg_t* image, unsigned row) {
  unsigned r = image->ring_rows ? (row % image->ring_rows) : row;
  return image->ptr + r * image->width * image->channels;
}

/// @} endgroup camera_isp_cfg

// ---------------------------- Capture API -------------------------------
/** 
 * @defgroup camera_isp_api Functions related to the Image Signal Processing (ISP) pipeline.
 * @{
 */

/**
 * @brief Captures frames until the AE is done or a maximum number of steps is reached.
 * This function can be called before `camera_isp_start_capture` to ensure the image is well-exposed.
 * It is optional and can be skipped if the user does not require auto-exposure 
 * or is willing to accept initial frames with incorrect exposure.
 * Has to be called after `camera_isp_coordinates_compute`.
 * 
 * @param c_cam camera channel
 * @param image image pointer and configuration
 */
void camera_isp_prepare_capture(chanend_t c_cam, image_cfg_t* image);

/**
 * @brief Sends the camera configuration to the ISP thread and starts capture process.
 * Capture process starts asynchronously, and the function returns immediately.
 * This function should be called after `camera_isp_coordinates_compute`.
 * To capture the image, the user should call `camera_isp_get_capture`.
 * 
 * @param c_cam camera channel
 * @param image image pointer and configuration
 */
void camera_isp_start_capture(chanend_t c_cam, image_cfg_t *image);

/**
 * @brief Receives an image from the ISP thread.
 * This function blocks until the image is ready. 
 * This function should be called after `camera_isp_start_capture`.
 * Image will be returned in the image structure passed to `camera_isp_start_capture`.
 * 
 * @param c_cam camera channel
 */
void camera_isp_get_capture(chanend_t c_cam);

/**
 * @brief Stops the camera sensor.
 * The sensor is stopped at the end of the current frame (between frames) and the
 * function returns once it is stopped, within one frame time. Use it to leave
 * streaming mode (`CONFIG_STREAMING`), for example to save power or before the
 * application exits. A later `camera_isp_start_capture` starts the sensor again.
 * In start/stop mode the sensor is already stopped after each capture and the
 * function returns immediately.
 * Must not be called while a capture is in progress (between
 * `camera_isp_start_capture` and `camera_isp_get_capture`).
 *
 * @param c_cam camera channel
 */
void camera_isp_stop(chanend_t c_cam);

/**
 * @brief Called from the ISP thread every time an output image row is complete.
 *
 * The default implementation does nothing. An application can provide its own
 * definition (the library one is weak) to forward rows to another consumer as
 * they are produced, for example a USB Video Class (UVC) thread, without
 * waiting for the whole frame.
 *
 * - Rows are delivered in order, from 0 to `image->height - 1`, for each captured frame.
 * - If a frame turns out to be incomplete (lines lost, e.g. while auto exposure
 *   runs in streaming mode), the ISP drops it and captures the next frame
 *   instead: rows then start again from 0. A row 0 arriving before the last row
 *   means the rows received so far belong to a dropped frame.
 * - The row is in the format of the capture mode (RAW8, RGB888 or YUV422) and,
 *   for RGB modes, already white balanced (if `CONFIG_APPLY_AWB` is enabled).
 * - The row data stays valid until the image buffer is reused by a new capture.
 * - It runs inside the ISP thread, so it must return quickly: the time spent
 *   here adds to the per-line processing budget (see the timing documentation).
 *   Copy the data or pass the pointer on, and do the heavy work elsewhere.
 *
 * @param image    image being captured (the one passed to `camera_isp_start_capture`)
 * @param row      output row index
 * @param row_data pointer to the row, `image->width * image->channels` bytes
 */
void camera_isp_line_callback(const image_cfg_t* image, unsigned row, const int8_t* row_data);

/**
 * @brief Main thread function for the ISP.
 * This function handles the interaction between the MIPI packet channel, 
 * control channel, and the camera channel. It processes incoming data 
 * and manages the ISP pipeline for image processing.
 *
 * @param c_pkt channel to receive mipi packets
 * @param c_ctrl channel to receive control messages from or to mipi
 * @param c_cam  channel array between user and isp
 */
void camera_isp_thread(
  streaming_chanend_t c_pkt,
  chanend_t c_ctrl,
  chanend_t c_cam);


// ---------------------------- Coordinates -------------------------------

/**
 * @brief compute MIPI coordinates, from user request to sensor dimensions.
 *
 * @param image_cfg pointer to the image configuration structure.
 */
void camera_isp_coordinates_compute(image_cfg_t* image_cfg);


// ---------------------------- RAW to RGB -------------------------------

/**
 * @brief Converts RAW8 lines from the sensor into an RAW8 image. 
 * Conversion: HxWx1 (RAW8) -> HxWx1 (RAW8) 
 * 
 * @param image structure containing image configuration and output pointer.
 * @param data_in pointer to the input RAW8 line data.
 * @param ln current sensor line number.
 */
void camera_isp_raw8_to_raw8(image_cfg_t* image, int8_t* data_in, unsigned ln);

/**
 * @brief Converts RAW8 lines into an RGB1 image. (image demosaicing)
 * Conversion: HxWx1 (RAW8) -> HxWx3 (RGB) 
 * 
 * @param image structure containing image configuration and output RGB pointer.
 * @param data_in pointer to the input RAW8 data.
 * @param ln current sensor line number.
 */
void camera_isp_raw8_to_rgb1(image_cfg_t* image, int8_t* data_in, unsigned ln);

/**
 * @brief Converts RAW8 lines into an RGB2 image. (downsampled by 2) 
 * Conversion: HxWx1 (RAW8) -> (H/2)x(W/2)x3 (RGB) 
 * 
 * @param image structure containing image configuration and output RGB pointer.
 * @param data_in pointer to the input RAW8 data.
 * @param ln current sensor line number.
 */
void camera_isp_raw8_to_rgb2(image_cfg_t* image, int8_t* data_in, unsigned ln);

/**
 * @brief Converts RAW8 lines into an RGB4 image. (downsampled by 4) 
 * Conversion: HxWx1 (RAW8) -> (H/4)x(W/4)x3 (RGB) 
 * 
 * @param image structure containing image configuration and output RGB pointer.
 * @param data_in pointer to the input RAW8 data.
 * @param ln current sensor line number.
 */
void camera_isp_raw8_to_rgb4(image_cfg_t* image, int8_t* data_in, unsigned ln);

/**
 * @brief Converts RAW8 lines into an YUV422 image. (downsampled by 2 horizontally)
 * 
 * @param image structure containing image configuration and output YUV pointer.
 * @param data_in pointer to the input RAW8 data.
 * @param ln current sensor line number.
 */
void camera_isp_raw8_to_yuv2(image_cfg_t* image, int8_t* data_in, unsigned ln);
// ---------------------------- White Balancing / Auto Exposure  -------------------------------

/**
 * @brief Applies Static White Balancing to the image.
 * This applies a gain to the R, G and B channels of the image.
 * Image must be in RGB format. Image pointer is updated with the new image.
 * @param image structure containing image configuration and output RGB pointer.
 */
void camera_isp_white_balance(image_cfg_t* image);

/**
 * @brief Applies Static White Balancing to some rows of the image.
 * Same as `camera_isp_white_balance`, restricted to `num_rows` rows starting at `row`.
 * Image must be in RGB format.
 * @param image structure containing image configuration and output RGB pointer.
 * @param row first output row to process.
 * @param num_rows number of rows to process.
 */
void camera_isp_white_balance_rows(image_cfg_t* image, unsigned row, unsigned num_rows);

/**
 * @brief Computes camera gain to apply for a new auto exposure step given an image. 
 * Computes the histograms and statistics of the image and computes the new exposure value.
 * It is based on false position method of histogram skewness.
 * It works well in unimodal distributions, but it is not very robust in multimodal distributions.  
 * @param image structure containing image configuration and output RGB pointer.
 * @return uint8_t new exposure value in [1, 80] or 0 if the exposure is already adjusted.
 */
unsigned camera_isp_auto_exposure(image_cfg_t* image);

/// @} endgroup camera_isp_api

C_API_END
