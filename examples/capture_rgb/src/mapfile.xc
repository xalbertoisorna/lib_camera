// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#include <xs1.h>
#include <platform.h>

#include "camera.h"

#define CAMERA_MIPI_TILE 1
#define CAMERA_I2C_TILE  0

extern "C" {
void user_app(chanend_t c_user_isp);
}

int main(void)
{
  chan c_cam;
  chan c_i2c;
  
  // Parallel jobs
  par{
    on tile[CAMERA_MIPI_TILE]: camera_main(c_cam, c_i2c);
    on tile[CAMERA_MIPI_TILE]: user_app(c_cam);
    on tile[CAMERA_I2C_TILE]: camera_control(c_i2c);
  }
  return 0;
}
