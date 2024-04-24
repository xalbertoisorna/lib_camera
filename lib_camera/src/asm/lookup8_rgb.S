// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/*
void lookup8_rgb(
    uint8_t* Y,
    const uint8_t* X,
    const uint8_t lut[3][256],
    const unsigned n_pix);
*/

#define FUNCTION_NAME lookup8_rgb

#define NSTACKWORDS  2
    
.text
.issue_mode  dual
.globl FUNCTION_NAME
.align 16
.type FUNCTION_NAME,@function
.cc_top FUNCTION_NAME.function,FUNCTION_NAME

#define Y       r0
#define X       r1
#define LUT     r2
#define n       r3

#define data    r4
#define i       r5

#define BLOCK_COUNT_LOG2 (3) //4
#define BLOCK_SIZE  (9)
#define BLOCK_RGB   {   add LUT, LUT, _512          ;   ld8u data, X[n]             };  \
                    {   sub n, n, 1                 ;   ld8u data, LUT[data]        };  \
                    st8 data, Y[n];                                                     \
                    {   sub LUT, LUT, _256          ;   ld8u data, X[n]             };  \
                    {   sub n, n, 1                 ;   ld8u data, LUT[data]        };  \
                    st8 data, Y[n];                                                     \
                    {   sub LUT, LUT, _256          ;   ld8u data, X[n]             };  \
                    {   sub n, n, 1                 ;   ld8u data, LUT[data]        };  \
                    st8 data, Y[n];


FUNCTION_NAME:
    dualentsp NSTACKWORDS
        std r4, r5, sp[0]

    //  We'll deal with the tail first, where tail is the first (n % BLOCK_COUNT)
    //  pixels. We deal with the tail be skipping (BLOCK_COUNT - (n % BLOCK_COUNT))
    //  blocks using a BRU instruction.

    //  i = BLOCK_SIZE * (BLOCK_COUNT - (n % BLOCK_COUNT))
    {   ldc i, (1<<BLOCK_COUNT_LOG2)    ;   mov r11, n                      }    
    {   zext r11, BLOCK_COUNT_LOG2      ;   sub X, X, 1                     }
    {   sub r11, i, r11                 ;   ldc i, BLOCK_SIZE               }
        mul i, r11, i

    {   ldc r11, 3                      ;   /*bf n, .Lfunc_end*/                }
    bf n, .Lfunc_end
        mul n, n, r11
#define _512    r5
#define _256    r11
        ldc _256, 256
    {   shl _512, _256, 1               ;   bru i                           }

#undef i

    .L_loop_top:
        // 16 blocks = 144 instructions
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
            //BLOCK_RGB
        {                               ;   bt n, .L_loop_top           }
    
.Lfunc_end:
    ldd r4, r5, sp[0]
    retsp NSTACKWORDS

    .cc_bottom FUNCTION_NAME.function
    .set FUNCTION_NAME.nstackwords,NSTACKWORDS
    .globl FUNCTION_NAME.nstackwords
    .set FUNCTION_NAME.maxcores,1
    .globl FUNCTION_NAME.maxcores
    .set FUNCTION_NAME.maxtimers,0
    .globl FUNCTION_NAME.maxtimers
    .set FUNCTION_NAME.maxchanends,0
    .globl FUNCTION_NAME.maxchanends
.Ltmp0:
    .size FUNCTION_NAME, .Ltmp0-FUNCTION_NAME
    .issue_mode  single
