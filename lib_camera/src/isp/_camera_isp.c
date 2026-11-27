// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <xcore/assert.h>
#include <xcore/select.h>
#include <xcore/hwtimer.h>
#include <xcore/assert.h>

// debug options
// (can be enabled via: -DDEBUG_PRINT_ENABLE_CAM_ISP=1)
#define DEBUG_UNIT CAM_ISP 
#include <debug_print.h>

#include "camera.h"
#include "camera_isp.h"
#include "camera_utils.h"
#include "camera_mipi.h"
#include "sensor_wrapper.h"

// -------- Globals & Constants -------------

// MIPI packet size
#define MIPI_MAX_PKT_SIZE_BYTES     ((SENSOR_WIDTH) + 4)
#define MIPI_PKT_BUFFER_COUNT       (4)

// Sensor State
static struct {
  unsigned frame_number;
  unsigned in_line_number;
  unsigned capture_pending;   // capture requested, waiting for the next frame start
  unsigned capturing;         // current frame is being captured
  unsigned sensor_streaming;  // sensor is out of standby
  unsigned frame_corrupt;     // invalid packet received during the current frame
  unsigned stop_pending;      // stop requested, sensor stops at the next end of frame
  unsigned ae_value;
} ph_state = {
    0,  // frame_number
    0,  // in_line_number
    0,  // capture_pending
    0,  // capturing
    0,  // sensor_streaming
    0,  // frame_corrupt
    0,  // stop_pending
    1,  // ae_value
};

// Sensor width maximum values
static const unsigned sensor_width_max_values[] = {
  [MODE_RAW]  = MODE_RAW_MAX_SIZE,
  [MODE_RGB1] = MODE_RGB1_MAX_SIZE,
  [MODE_RGB2] = MODE_RGB2_MAX_SIZE,
  [MODE_RGB4] = MODE_RGB4_MAX_SIZE,
  [MODE_YUV2] = MODE_YUV2_MAX_SIZE,
};

// Sensor scale values
static const unsigned sensor_scale_values[] = {
  [MODE_RAW]  = 1,
  [MODE_RGB1] = 1,
  [MODE_RGB2] = 2,
  [MODE_RGB4] = 4,
  [MODE_YUV2] = 2,
};

// MIPI packet header
typedef struct {
  mipi_header_t header;                       // MIPI header 
  uint8_t payload[MIPI_MAX_PKT_SIZE_BYTES];   // MIPI payload data
} mipi_packet_t;

// -------- Image API -------------------
void camera_isp_prepare_capture(chanend_t c_cam, image_cfg_t* image)
{
  const unsigned max_steps = 20;
  for (unsigned i = 0; i < max_steps; i++) {
    camera_isp_start_capture(c_cam, image);
    camera_isp_get_capture(c_cam);
    if (!ph_state.ae_value) {
      break;
    }
  }
}
 
void camera_isp_start_capture(chanend_t c_cam, image_cfg_t *image) {
  chan_out_buf_byte(c_cam, (uint8_t*)image, sizeof(image_cfg_t));
}
 
void camera_isp_get_capture(chanend_t c_cam) {
  chan_in_byte(c_cam);
}

void camera_isp_stop(chanend_t c_cam) {
  image_cfg_t stop_request = {0};   // NULL image pointer means stop
  chan_out_buf_byte(c_cam, (uint8_t*)&stop_request, sizeof(image_cfg_t));
  chan_in_byte(c_cam);
}


// -------- State handlers --------

static
void handle_unknown_packet(
  mipi_data_type_t data_type) {
  // Other CSI-2 packet types (e.g. embedded data) are ignored. An invalid
  // header means the receiver lost sync (e.g. the ISP thread was busy while the
  // sensor kept streaming): drop the current frame, the receiver resyncs on the
  // next packet.
  if (data_type == 0x3F) {
    ph_state.frame_corrupt = 1;
  }
}

static
void handle_post_process(image_cfg_t* image)
{
  // Image pointer could be NULL if EOF is reached before asking a picture
  if (image->ptr == NULL) {
    return;
  }

  // Note: white balance is applied per row, as rows are completed (see handle_rows_ready)

#if (CONFIG_APPLY_AE)
  // auto exposure statistics support RGB and YUV images only (not RAW)
  if (image->config->mode != MODE_RAW) {
    ph_state.ae_value = camera_isp_auto_exposure(image);
    if (ph_state.ae_value) {
      camera_sensor_set_exposure(ph_state.ae_value);
    }
  }
#endif
}

static
void handle_end_of_frame(
  image_cfg_t* image,
  chanend_t c_cam)
{
  if (!ph_state.capturing) {
    return; // frame not requested by the user
  }
  ph_state.capturing = 0;

  // Only deliver complete frames. Lines can be lost if the ISP thread was busy
  // when they arrived (e.g. auto exposure computation overlapping the next
  // frame in streaming mode). In that case retry on the next frame.
  if (ph_state.in_line_number != SENSOR_HEIGHT || ph_state.frame_corrupt) {
    debug_printf("Incomplete frame (%d lines), retrying\n", ph_state.in_line_number);
    ph_state.capture_pending = 1;
    return;
  }

  handle_post_process(image);

#if (!CONFIG_STREAMING)
  camera_sensor_stop();
  ph_state.sensor_streaming = 0;
#endif

  chan_out_byte(c_cam, 1);
}

// Default line callback: does nothing. Applications can override it.
__attribute__((weak))
void camera_isp_line_callback(const image_cfg_t* image, unsigned row, const int8_t* row_data)
{
  (void)image;
  (void)row;
  (void)row_data;
}

// Called after a sensor line has been converted. Converters complete output
// rows at fixed points (every 1, 2 or 4 sensor lines depending on the mode).
static
void handle_rows_ready(image_cfg_t* image, unsigned roi_ln)
{
  unsigned first_row, num_rows;
  switch (image->config->mode) {
    case MODE_RAW:  first_row = roi_ln;               num_rows = 1; break;
    case MODE_RGB1: if ((roi_ln & 1) != 1) return; first_row = roi_ln - 1;        num_rows = 2; break;
    case MODE_RGB2: if ((roi_ln & 3) != 3) return; first_row = (roi_ln - 3) >> 1; num_rows = 2; break;
    case MODE_RGB4: if ((roi_ln & 3) != 3) return; first_row = (roi_ln - 3) >> 2; num_rows = 1; break;
    case MODE_YUV2: if ((roi_ln & 1) != 1) return; first_row = (roi_ln - 1) >> 1; num_rows = 1; break;
    default: return;
  }

#if (CONFIG_APPLY_AWB)
  // white balance kernel operates on RGB888 pixels only (not RAW or YUV)
  camera_mode_t mode = image->config->mode;
  if (mode == MODE_RGB1 || mode == MODE_RGB2 || mode == MODE_RGB4) {
    camera_isp_white_balance_rows(image, first_row, num_rows);
  }
#endif

  const unsigned row_len = image->width * image->channels;
  for (unsigned r = first_row; r < first_row + num_rows; r++) {
    camera_isp_line_callback(image, r, image->ptr + (r * row_len));
  }
}

static
void handle_expected_lines(image_cfg_t* image, int8_t* data_in) {
  unsigned ln = ph_state.in_line_number;
  camera_mode_t mode = image->config->mode;
  
  // Check if the line is inside the region of interest
  if (ln < image->config->y1 || ln >= image->config->y2) {
    return;
  }
  
  // Provide the image data to the user
  switch (mode)
  {
    case MODE_RAW:{
      camera_isp_raw8_to_raw8(image, data_in, ln);
      break;
    }
    case MODE_RGB1:{
      camera_isp_raw8_to_rgb1(image, data_in, ln);
      break;
    }
    case MODE_RGB2:{
      camera_isp_raw8_to_rgb2(image, data_in, ln);
      break;
    }
    case MODE_RGB4:{
      camera_isp_raw8_to_rgb4(image, data_in, ln);
      break;
    }
    case MODE_YUV2:{
      camera_isp_raw8_to_yuv2(image, data_in, ln);
      break;
    }
    default:{
      xassert(0 && "mode not supported");
      break;
    }
  }

  handle_rows_ready(image, ln - image->config->y1);
}



// -------- Image Coordinates --------

void camera_isp_coordinates_compute(image_cfg_t* img_cfg){
  camera_cfg_t *cfg = img_cfg->config;

  // If RAW, scale = 1
  unsigned mode = cfg->mode;
  unsigned max_size = sensor_width_max_values[mode];
  unsigned scale = sensor_scale_values[mode];

  // Compute the coordinates of the region of interest
  cfg->x1 = cfg->offset_x * SENSOR_WIDTH;
  cfg->y1 = cfg->offset_y * SENSOR_HEIGHT;

  // ensure all are even and unsigned
  cfg->x1 = ((unsigned)cfg->x1) & ~3;
  cfg->y1 = ((unsigned)cfg->y1) & ~1;

  cfg->x2 = cfg->x1 + img_cfg->width * scale;
  cfg->y2 = cfg->y1 + img_cfg->height * scale;

  cfg->x2 = ((unsigned)cfg->x2) & ~1;
  cfg->y2 = ((unsigned)cfg->y2) & ~1;

  // if flip vertical add one to y1 and y2
  if (CONFIG_FLIP == FLIP_VERTICAL) {
    cfg->y1 += 1;
    cfg->y2 += 1;
  }

  // compute sensor width and height
  cfg->sensor_width = cfg->x2 - cfg->x1;
  cfg->sensor_height = cfg->y2 - cfg->y1;

  // if raw ensure channels are 1, else 3
  unsigned cond_raw = (mode == MODE_RAW && img_cfg->channels == 1);
  unsigned cond_yuv = (mode == MODE_YUV2 && img_cfg->channels == 2);
  unsigned cond_rgb = ((!cond_raw) && (!cond_yuv) && img_cfg->channels == 3);

  // debug info
  debug_printf("Coords: x1:%d, y1:%d, x2:%d, y2:%d\n", cfg->x1, cfg->y1, cfg->x2, cfg->y2);
  debug_printf("Sensor: w:%d, h:%d\n", cfg->sensor_width, cfg->sensor_height);
  debug_printf("Mode: %d\n", mode);

  // ensure everything is logical
  xassert(cond_raw || cond_rgb || cond_yuv && "channels not valid");
  xassert(cfg->sensor_width <= max_size && "sensor_width");
  xassert(cfg->x1 < cfg->x2 && "x1");
  xassert(cfg->y1 < cfg->y2 && "y1");
  xassert(cfg->x2 <= SENSOR_WIDTH && "x2");
  xassert(cfg->y2 <= SENSOR_HEIGHT && "y2");
  xassert((img_cfg->width % 4) == 0 && "width has to be divisible by 4");
  xassert((img_cfg->height % 4) == 0 && "height has to be divisible by 4");
}


// -------- Frame handling --------------

static
void camera_isp_packet_handler(
  const mipi_packet_t* pkt,
  image_cfg_t* image_cfg,
  chanend_t c_isp_to_user) {

  // Definitions
  const mipi_header_t header = pkt->header;
  const mipi_data_type_t data_type = MIPI_GET_DATA_TYPE(header);

  // Timing
  static uint32_t t_init=0;
  static uint32_t t_end=0;

  // Data pointers calculation
  int8_t* data_in = (int8_t*)(&pkt->payload[0]);


  // Handle packets depending on their type
  switch (data_type) {
    case MIPI_DT_FRAME_START:
      t_init = get_reference_time();
      ph_state.in_line_number = 0;
      ph_state.frame_corrupt = 0;
      ph_state.frame_number++;
      // a capture always starts at the beginning of a frame
      if (ph_state.capture_pending) {
        ph_state.capture_pending = 0;
        ph_state.capturing = 1;
      }
      break;

    case MIPI_DT_RAW8:
      if (ph_state.capturing && ph_state.in_line_number < SENSOR_HEIGHT) {
        handle_expected_lines(image_cfg, data_in);
      }
      ph_state.in_line_number++;
      break;

    case MIPI_DT_FRAME_END:
      t_end = get_reference_time();
      debug_printf("Frame time: %d cycles\n", t_end - t_init);
      handle_end_of_frame(image_cfg, c_isp_to_user);
      // stop between frames, so the next start begins with a clean frame
      if (ph_state.stop_pending) {
        camera_sensor_stop();
        ph_state.sensor_streaming = 0;
        ph_state.stop_pending = 0;
        chan_out_byte(c_isp_to_user, 1);
      }
      break;

    default:
      handle_unknown_packet(data_type);
      break;
  }
  return;
}


// -------- Main packet handler thread --------

void camera_isp_thread(
  streaming_chanend_t c_pkt,
  chanend_t c_ctrl,
  chanend_t c_cam) {

  mipi_packet_t ALIGNED_8 packet_buffer[MIPI_PKT_BUFFER_COUNT];
  mipi_packet_t* pkt;
  unsigned pkt_idx = 0;

  // Image configuration
  image_cfg_t image;
  image.ptr = NULL;

  // Sensor configuration (the sensor is left streaming after init)
  camera_sensor_init();
  ph_state.sensor_streaming = 1;

  // Wait for the sensor to start
  delay_milliseconds_cpp(600);

  // Give the MIPI packet receiver a first buffer
  s_chan_out_word(c_pkt, (unsigned)&packet_buffer[pkt_idx]);


  SELECT_RES(
    CASE_THEN(c_pkt, on_c_pkt_change),
    CASE_THEN(c_cam, on_c_user_isp_change)) {
  on_c_pkt_change: { // attending mipi_packet_rx
    pkt = (mipi_packet_t*)s_chan_in_word(c_pkt);
    pkt_idx = (pkt_idx + 1) & (MIPI_PKT_BUFFER_COUNT - 1);
    s_chan_out_word(c_pkt, (unsigned)&packet_buffer[pkt_idx]);
    camera_isp_packet_handler(pkt, &image, c_cam);
    continue;
    }
  on_c_user_isp_change: { // attending user_app
    // user petition
    chan_in_buf_byte(c_cam, (uint8_t*)&image, sizeof(image_cfg_t)); // recieve info from user
    if (image.ptr == NULL) {                // stop request (camera_isp_stop)
      ph_state.capture_pending = 0;
      if (ph_state.sensor_streaming) {
        ph_state.stop_pending = 1;          // acknowledged at the next end of frame
      } else {
        chan_out_byte(c_cam, 1);            // already stopped
      }
      continue;
    }
    ph_state.capture_pending = 1;
    // Start camera (in streaming mode the sensor is never stopped)
    if (!ph_state.sensor_streaming) {
      camera_sensor_start();
      ph_state.sensor_streaming = 1;
    }
    continue;
    }
  }
}
