#pragma once
#define XSTR(x) STR(x)
#define STR(x) #x

// -------------- Sensor abstraction layer. --------------
#define CONFIG_IMX219_SUPPORT 1
#define CONFIG_GC2145_SUPPORT 0
#define CONFIG_MODE 0


#if CONFIG_IMX219_SUPPORT
#include "imx219.h"
#endif

#if CONFIG_GC2145_SUPPORT
#include "gc2145.h"
#endif

// Modes definition
#if CONFIG_MODE == 0
    #define EXPECTED_FORMAT MIPI_DT_RAW10   // RAW 10 bit data identifier
    #define MIPI_IMAGE_WIDTH_PIXELS         640 // csi2 packed (stride 800) 
    #define MIPI_IMAGE_HEIGHT_PIXELS        480
#elif CONFIG_MODE == 1
    #define EXPECTED_FORMAT MIPI_DT_YUV422_8BIT
    #define MIPI_IMAGE_WIDTH_PIXELS     1600
    #define MIPI_IMAGE_HEIGHT_PIXELS    1200
#elif CONFIG_MODE == 2
    #define EXPECTED_FORMAT MIPI_DT_RAW10   // RAW 10 bit data identifier
    #define MIPI_IMAGE_WIDTH_PIXELS         1640 // csi2 packed (stride 800) 
    #define MIPI_IMAGE_HEIGHT_PIXELS        1232
#endif

// Camera functions to be called from main program
#define camera_init(x)                  imx219_init(x)
#define camera_start(x)                 imx219_stream_start(x)
#define camera_configure(x)             imx219_configure_mode(x)


// -------------------------------------------------------------------------------
// Camera dependant (do not edit)
#if EXPECTED_FORMAT == MIPI_DT_RAW10
    #define MIPI_IMAGE_WIDTH_BYTES ((MIPI_IMAGE_WIDTH_PIXELS) >> 2) * 5 // by 5/4
#elif EXPECTED_FORMAT == MIPI_DT_RAW8
    #define MIPI_IMAGE_WIDTH_BYTES MIPI_IMAGE_WIDTH_PIXELS // same size
#endif

#define MIPI_LINE_WIDTH_BYTES MIPI_IMAGE_WIDTH_BYTES
#define MIPI_PKT_BUFFER_COUNT 4 // this is user defined
#define MIPI_MAX_PKT_SIZE_BYTES ((MIPI_LINE_WIDTH_BYTES) + 4)
#define MIPI_TILE 1



// SRAM Image storage (do not edit)
//TODO check maximum storage size for the image
#define MAX_MEMORY_SIZE 500000 << 2 //becasue half needed is code

#if MIPI_IMAGE_WIDTH_BYTES*MIPI_IMAGE_HEIGHT_PIXELS > MAX_MEMORY_SIZE
    #error "size of the image does not fit in internal ram"
#endif

extern uint8_t FINAL_IMAGE[MIPI_IMAGE_HEIGHT_PIXELS][MIPI_LINE_WIDTH_BYTES];

