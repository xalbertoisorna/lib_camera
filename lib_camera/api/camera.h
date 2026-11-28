// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#include "api.h"
#include "xcore_compat.h"    // chanend_t safe use from XC
#include "sensor_wrapper.h"  // sensor configuration


/**
 * @defgroup camera_main Main camera functions
 * @{
 */

// High-Level Sensor Configuration
#define SENSOR_WIDTH    800 ///< Sensor width in pixels
#define SENSOR_HEIGHT   800 ///< Sensor height in pixels

#define CONFIG_FLIP         FLIP_NONE ///< Flip mode: FLIP_NONE, FLIP_VERTICAL
#define CONFIG_BINNING      BINNING_ON  ///< Binning mode: BINNING_ON or BINNING_OFF
#define CONFIG_CENTRALISE   CENTRALISE_ON ///< Centralise mode: CENTRALISE_ON or CENTRALISE_OFF

#ifndef CONFIG_APPLY_AWB
#define CONFIG_APPLY_AWB    (1) ///< Apply White Balance: 0 skip, 1 apply
#endif

#ifndef CONFIG_APPLY_AE
#define CONFIG_APPLY_AE     (1) ///< Apply Auto Exposure: 0 to skip, 1 apply once, 2 continuous 
#endif

#if defined(__XC__)

/**
 * @brief Main entry point for the lib_camera module.
 *
 * This function initializes and configures the MIPI interface, and starts both 
 * the MIPI RX and ISP processing threads. 
 * Usable when MIPI and I2C are on the same or different tiles.
 * It can be called from XC only.
 * 
 * @param c_camera Channel endpoint for communication with the user application.
 * @return void
 * @note This function can only be called from XC code.
 * @note use camera_main_single_tile() for single-tile configurations.
 */
void camera_main(chanend_t c_camera);

#endif // defined(__XC__)


#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

/**
 * @brief Main entry point for the lib_camera module (single tile variant).
 *
 * This function initializes and configures the MIPI interface, and starts both 
 * the MIPI RX and ISP processing threads. 
 * This variant is optimized for configurations where both MIPI and I2C 
 * functionalities are on the same tile.
 * 
 * @param c_camera Channel endpoint for communication with the user application.
 * @return void
 * @note This function can be called from both C and XC code.
 * @note use camera_main() for multi-tile configurations.
 */
void camera_main_single_tile(chanend_t c_camera);

#if defined(__cplusplus) || defined(__XC__)
}
#endif

/// @} endgroup camera_main
