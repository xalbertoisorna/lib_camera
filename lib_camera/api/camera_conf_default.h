// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @file camera_conf_default.h
 * @brief Default build-time configuration for lib_camera.
 *
 * Any of the settings below can be overridden by the application in one of
 * two ways:
 *
 * 1. Providing an optional header named ``camera_conf.h`` in the application
 *    include path. XCommon CMake detects it (``LIB_OPTIONAL_HEADERS``) and
 *    defines ``__camera_conf_h_exists__``, so it is included here before the
 *    defaults are applied.
 * 2. Passing a compiler definition from the application ``CMakeLists.txt``,
 *    e.g. ``-DCONFIG_APPLY_AE=2``.
 *
 * Use only one of the two methods for a given setting, otherwise the compiler
 * will report a macro redefinition.
 */

#pragma once

#include "sensor_wrapper.h"  // binning_t, centralise_t, orientation_t

#ifdef __camera_conf_h_exists__
#include "camera_conf.h"
#endif

/**
 * @addtogroup camera_main
 * @{
 */

// High-Level Sensor Configuration
#ifndef SENSOR_WIDTH
#define SENSOR_WIDTH        800 ///< Sensor width in pixels
#endif

#ifndef SENSOR_HEIGHT
#define SENSOR_HEIGHT       800 ///< Sensor height in pixels
#endif

#ifndef CONFIG_FLIP
#define CONFIG_FLIP         FLIP_NONE ///< Flip mode: FLIP_NONE, FLIP_VERTICAL
#endif

#ifndef CONFIG_BINNING
#define CONFIG_BINNING      BINNING_ON ///< Binning mode: BINNING_ON or BINNING_OFF
#endif

#ifndef CONFIG_CENTRALISE
#define CONFIG_CENTRALISE   CENTRALISE_ON ///< Centralise mode: CENTRALISE_ON or CENTRALISE_OFF
#endif

#ifndef CONFIG_APPLY_AWB
#define CONFIG_APPLY_AWB    (1) ///< Apply White Balance: 0 skip, 1 apply
#endif

#ifndef CONFIG_APPLY_AE
#define CONFIG_APPLY_AE     (1) ///< Apply Auto Exposure: 0 to skip, 1 apply once, 2 continuous
#endif

#ifndef CONFIG_STREAMING
#define CONFIG_STREAMING    (0) ///< Sensor mode: 0 start/stop the sensor per capture, 1 continuous streaming
#endif

#ifndef CONFIG_STREAMING_FPS
#define CONFIG_STREAMING_FPS (30) ///< Target frame rate in streaming mode (caps the exposure time)
#endif

#ifndef CONFIG_STREAMING_PCK_DIV
#define CONFIG_STREAMING_PCK_DIV (5) ///< Sensor pixel clock divider in streaming mode (VTPXCK_DIV: 4, 5, 8 or 10). 5 reads a line in 29.9 us, 10 in 59.9 us. 10 stops the image output for windows wider than 800: use CONFIG_STREAMING_LINE_LENGTH for slower lines
#endif

#ifndef CONFIG_STREAMING_LINE_LENGTH
#define CONFIG_STREAMING_LINE_LENGTH (3448) ///< Sensor line length in pixel clocks in streaming mode (line_length_pck, >= 3448). The line time scales with it: 6896 doubles it
#endif

/// @} addtogroup camera_main
