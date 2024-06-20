// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>

#include "api.h"
#include "xcore_compat.h"
#include "camera_mipi_defines.h"
#include "lib_camera.h"

C_API_START

// this struct will hold the configuration for the camera
typedef struct
{
  // Offsets
  unsigned offset_x;
  unsigned offset_y;
  // Scale
  float sx;
  float sy;
  // Shear
  float shx;
  float shy;
  // Rotation
  float angle;
  // Transform
  float* T;
} camera_configure_t;


// this will hold the image data and possible configurations
// we could split this into two structs, but for now we will keep it simple
// splitting will just cause have more functions, so is a balance to decide
typedef struct {
  unsigned height;
  unsigned width;
  unsigned channels;
  int8_t* ptr;
  camera_configure_t* config;
} Image_cfg_t;


void camera_isp_send_cfg(chanend_t c_user, Image_cfg_t* image);

void camera_isp_recv_cfg(chanend_t c_user, Image_cfg_t* image);

void camera_isp_send_ctrl(chanend_t c_ctrl, sensor_control_t* ctrl);

void camera_isp_recv_ctrl(chanend_t c_user, sensor_control_t* ctrl);

void camera_isp_thread(
  streaming_chanend_t c_pkt,
  streaming_chanend_t c_ctrl,
  chanend_t c_control,
  chanend_t c_cam[N_CH_USER_ISP]);


C_API_END
