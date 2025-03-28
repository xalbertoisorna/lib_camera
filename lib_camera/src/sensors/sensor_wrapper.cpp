// Copyright 2023-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <assert.h>

#include <platform.h>
#include <xcore/assert.h>
#include <xcore/select.h>
#include <xcore/chanend.h>

#include "print.h"

#include "camera.h"
#include "camera_mipi.h"
#include "sensor_wrapper.h"
#include "sensor_base.hpp"
#include "camera_utils.h"

#include "imx219.hpp"

using namespace sensor;

// Global sensor object pointer
IMX219* camera_sensor_ptr = nullptr;
i2c_master_t i2c_ctx;
i2c_config_t i2c_conf;

void camera_sensor_init() {
  // I2C settings
  i2c_conf.device_addr = I2C_DEV_ADDR;
  i2c_conf.speed = I2C_DEV_SPEED;
  i2c_conf.p_scl = PORT_I2C_SCL;
  i2c_conf.p_sda = PORT_I2C_SDA;
  i2c_conf.i2c_ctx_ptr = &i2c_ctx;

  // Sensor settings
  resolution_t res = {
    .sensor_width = SENSOR_WIDHT,
    .sensor_height = SENSOR_HEIGHT
  };
  
  // Global sensor object
  camera_sensor_ptr = new IMX219(
    (i2c_config_t)i2c_conf, 
    (resolution_t)res, 
    (pixel_format_t)MIPI_DT_RAW8, 
    (binning_t)CONFIG_BINNING, 
    (centralise_t)CONFIG_CENTRALISE);

  // Init the I2C sensor first configuration
  printstr("Camera init\n");
  int ret = 0;
  ret |= camera_sensor_ptr->initialize();
  delay_milliseconds_cpp(500);
  ret |= camera_sensor_ptr->configure();
  delay_milliseconds_cpp(500);
  ret |= camera_sensor_ptr->stream_start();
  delay_milliseconds_cpp(500);
  xassert((ret == 0) && "Could not initialise camera");
  printstr("Camera_started and configured\n");
}

void camera_sensor_start() {
  int ret = camera_sensor_ptr->stream_start();
  xassert((ret == 0) && "Could not start camera");
}

void camera_sensor_stop() {
  int ret = camera_sensor_ptr->stream_stop();
  xassert((ret == 0) && "Could not stop camera");
}

void camera_sensor_set_tp(uint16_t pattern){
  int ret = camera_sensor_ptr->set_test_pattern(pattern);
  xassert((ret == 0) && "Could not set test pattern");
}

void camera_control(chanend_t c_control){
  // store the response
  uint32_t encoded_response;
  sensor_control_t cmd;
  uint8_t arg;
  int ret = 0;

  // read the response from the channel
  SELECT_RES(
    CASE_THEN(c_control, c_control_handler))
    {
      c_control_handler: {
        encoded_response = chan_in_word(c_control);
        cmd = (sensor_control_t)DECODE_CMD(encoded_response);
        arg = DECODE_ARG(encoded_response);
        printf("Received command: %d, arg: %d\n", cmd, arg);
        switch (cmd)
        {
        case SENSOR_INIT:
          camera_sensor_init(); break; 
        case SENSOR_STREAM_START:
          camera_sensor_start(); break;
        case SENSOR_STREAM_STOP:
          camera_sensor_stop(); break;
        case SENSOR_SET_EXPOSURE:
          printf("Not implemented yet\n");
        default:
          break;
        }
        assert((ret == 0) && "Could not perform I2C write");
      }
    }
}

void camera_sensor_control_tx(chanend_t c_control, sensor_control_t cmd, uint8_t arg) {
  // Encode the command and argument
  uint32_t encoded_command = ENCODE_CTRL(cmd, arg); 
  // Send the command to the channel
  chan_out_word(c_control, encoded_command);
}
