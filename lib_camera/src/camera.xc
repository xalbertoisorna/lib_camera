#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <platform.h>
#include <xs1.h>

#include "xcore_compat.h"    // chanend_t safe use from XC

#define CAM_MIPI_TILE_NUM (1)
#define CAM_I2C_TILE_NUM (1)

#if (CAM_MIPI_TILE_NUM == CAM_I2C_TILE_NUM)

extern "C" {
    void camera_main_single_tile(chanend_t c_camera);
}

void camera_main(chanend_t c_cam){
    camera_main_single_tile(c_cam);
}

#else // MIPI and I2C on different tiles

extern "C" {
    extern void camera_main_tile_mipi(chanend_t c_cam, chanend_t c_i2c);
    extern void camera_main_tile_i2c(chanend_t c_i2c);
}

void camera_main(chanend_t c_cam){
    chan c_i2c;
    par{
        on tile[CAM_I2C_TILE_NUM]: camera_main_tile_i2c(c_i2c);
        on tile[CAM_MIPI_TILE_NUM]: camera_main_tile_mipi(c_cam, c_i2c);
    } 
}

#endif
