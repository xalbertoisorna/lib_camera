// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#include <xs1.h>
#include <platform.h>

#include "camera.h"

extern "C" {
void user_app(chanend_t c_user_isp, streaming chanend c_usb);
}

int main(void)
{
  chan c_cam;
  streaming chan c_usb;
  
  // Parallel jobs
  par{
    on tile[1]: camera_main(c_cam);
    on tile[1]: user_app(c_cam, c_usb);
  }
  return 0;
}
