// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <xs3a_registers.h>

.issue_mode dual

// void resize_loop2( int8_t * WXYZ,  -> r0
//                    int8_t * a,     -> r1
//                    int8_t * b,     -> r2
//                    int8_t * c,     -> r3
//                    int8_t * d,     
//                    int8_t * out)

#define FUNCTION_NAME   resize_loop2
#define NSTACKWORDS     (8 + 8)

#define STACK_D_PTR     (NSTACKWORDS + 1)
#define STACK_OUT_PTR   (NSTACKWORDS + 2)
#define STACK_VEC_TMP   (NSTACKWORDS - 8)

.globl FUNCTION_NAME.nstackwords
.globl FUNCTION_NAME.maxthreads
.globl FUNCTION_NAME.maxtimers
.globl FUNCTION_NAME.maxchanends

.linkset FUNCTION_NAME.nstackwords, NSTACKWORDS
.linkset FUNCTION_NAME.maxchanends, 0
.linkset FUNCTION_NAME.maxtimers,   0
.linkset FUNCTION_NAME.maxthreads,  0

.globl FUNCTION_NAME
.type FUNCTION_NAME, @function
.text
.cc_top FUNCTION_NAME.func, FUNCTION_NAME

#define WXYZ r0
#define a_ptr r1
#define b_ptr r2
#define c_ptr r3
#define d_ptr r4
#define t0 r5
#define t1 r6
#define t2 r7

.align 4
FUNCTION_NAME:
  dualentsp NSTACKWORDS
  std r4, r5, sp[1]
  std r6, r7, sp[2]
  std r8, r9, sp[3]

  ldc r11, 0x200

  { ldaw r8, sp[STACK_VEC_TMP]  ; vsetc r11                   }
  { ldc r11, 0                  ; ldw d_ptr, sp[STACK_D_PTR]  }
  vclrdr
  vstr r8[0] // clear the stack memory

  {                             ; ld8u t0, WXYZ[r11]          }
  { add t1, t0, 0               ; shl t2, t0, 16              }
  zip t1, t0, 3
  { or t0, t0, t2               ; vldc a_ptr[0]               }
  { add r11, r11, 1             ; stw t0, r8[0]               }
  {                             ; vlmacc r8[0]                }

  {                             ; ld8u t0, WXYZ[r11]          }
  { add t1, t0, 0               ; shl t2, t0, 16              }
  zip t1, t0, 3
  { or t0, t0, t2               ; vldc b_ptr[0]               }
  { add r11, r11, 1             ; stw t0, r8[0]               }
  {                             ; vlmacc r8[0]                }

  {                             ; ld8u t0, WXYZ[r11]          }
  { add t1, t0, 0               ; shl t2, t0, 16              }
  zip t1, t0, 3
  { or t0, t0, t2               ; vldc c_ptr[0]               }
  { add r11, r11, 1             ; stw t0, r8[0]               }
  {                             ; vlmacc r8[0]                }

  {                             ; ld8u t0, WXYZ[r11]          }
  { add t1, t0, 0               ; shl t2, t0, 16              }
  zip t1, t0, 3
  { or t0, t0, t2               ; vldc d_ptr[0]               }
  { mkmsk r11, 3                ; stw t0, r8[0]               }
  { ldaw r0, sp[STACK_OUT_PTR]  ; vlmacc r8[0]                }

  // now has the red, green and blue
  // we assume that a * W + b * X + c * Y + d * Z never overflows int8
  // so no saturation
  vstrpv r0[0], r11

  ldd r8, r9, sp[3]
  ldd r6, r7, sp[2]
  ldd r4, r5, sp[1]
  retsp NSTACKWORDS

.size FUNCTION_NAME, .-FUNCTION_NAME
.cc_bottom FUNCTION_NAME.func
