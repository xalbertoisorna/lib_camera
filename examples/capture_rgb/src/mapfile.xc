// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#include <xs1.h>
#include <platform.h>
#include <xscope.h>

#include "camera.h"

extern "C" {
#include "xscope_io_device.h"
}

extern "C" {
void user_app(chanend_t c_user_isp);
}

int main(void)
{
  // xscope
  chan xscope_chan;
  chan c_cam;
  
  // Parallel jobs
  par{
    xscope_host_data(xscope_chan);
    on tile[1]: xscope_io_init(xscope_chan);
    on tile[1]: camera_main(c_cam);
    on tile[1]: user_app(c_cam);
  }
  return 0;
}
