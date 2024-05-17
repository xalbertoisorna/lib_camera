// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <xs3a_registers.h>

.issue_mode dual

// void resize_loop1( int8_t * WXYZ,  -> r0
//                    int8_t * a,     -> r1
//                    int8_t * b,     -> r2
//                    int8_t * c,     -> r3
//                    int8_t * d,     
//                    int8_t * out)

#define FUNCTION_NAME   resize_loop1
#define NSTACKWORDS     (8 + 10)

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

  { ldaw r11, sp[STACK_VEC_TMP] ; vsetc r11                   }
  { vldc r0[0]                  ; ldc r0, 0                   }
  {                             ; ldw d_ptr, sp[STACK_D_PTR]  }
  vclrdr
  { sub r11, r11, 4             ; vstr r11[0]                 }

  {                             ; ld8u t0, c_ptr[r0]          }
  { shl t0, t0, 8               ; ld8u t1, d_ptr[r0]          }
  { or t0, t0, t1               ; ld8u t1, b_ptr[r0]          }
  { shl t1, t1, 16              ; ld8u t2, a_ptr[r0]          }
  { or t0, t0, t1               ; shl t2, t2, 24              }
  { or t0, t0, t2               ; add r0, r0, 1               }
  {                             ; stw t0, r11[0]              }
  {                             ; vlmaccr r11[0]              }
  // now has the red

  {                             ; ld8u t0, c_ptr[r0]          }
  { shl t0, t0, 8               ; ld8u t1, d_ptr[r0]          }
  { or t0, t0, t1               ; ld8u t1, b_ptr[r0]          }
  { shl t1, t1, 16              ; ld8u t2, a_ptr[r0]          }
  { or t0, t0, t1               ; shl t2, t2, 24              }
  { or t0, t0, t2               ; add r0, r0, 1               }
  {                             ; stw t0, r11[0]              }
  {                             ; vlmaccr r11[0]              }
  // now has the red and green

  {                             ; ld8u t0, c_ptr[r0]          }
  { shl t0, t0, 8               ; ld8u t1, d_ptr[r0]          }
  { or t0, t0, t1               ; ld8u t1, b_ptr[r0]          }
  { shl t1, t1, 16              ; ld8u t2, a_ptr[r0]          }
  { or t0, t0, t1               ; shl t2, t2, 24              }
  { or t0, t0, t2               ; ldw r6, sp[STACK_OUT_PTR]   }
  {                             ; stw t0, r11[0]              }
  { add r11, r11, 4             ; vlmaccr r11[0]              }
  // now has the red, green and blue

  {                             ; vlsat r11[0]                }
  {                             ; vstr r11[0]                 }
  { sub r7, r0, 1               ; ld8u r5, r11[r0]            }
  st8 r5, r6[r0]

  { sub r0, r7, 1               ; ld8u r5, r11[r7]            }
  st8 r5, r6[r7]

  {                             ; ld8u r5, r11[r0]            }
  st8 r5, r6[r0]

  ldd r8, r9, sp[3]
  ldd r6, r7, sp[2]
  ldd r4, r5, sp[1]
  retsp NSTACKWORDS

.size FUNCTION_NAME, .-FUNCTION_NAME
.cc_bottom FUNCTION_NAME.func
