// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#include "api.h"
#include "xcore_compat.h"    // chanend_t safe use from XC
#include "sensor_wrapper.h"  // sensor configuration
#include "camera_conf_default.h" // build-time configuration (overridable)

C_API_START

/**
 * @defgroup camera_main Main camera functions
 * @{
 */

/**
 * @brief Main entry point for the lib_camera module.
 *
 * This function initializes and configures the MIPI interface, and starts both 
 * the MIPI RX and ISP processing threads. 
 * 
 * @param c_camera  Channel endpoint for communication with the user application.
 */
void camera_main(chanend_t c_camera);

/// @} endgroup camera_main

C_API_END
