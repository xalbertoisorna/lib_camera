// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef ALIGNED_4
#define ALIGNED_4 __attribute__((aligned(4)))
#endif

extern const int8_t *kernels_group[16];
extern const int8_t *remainders_group[16];
extern const int8_t vcrem[32];
extern const int8_t yuv_adds[32];
extern const int16_t yuv_vsat[16];
