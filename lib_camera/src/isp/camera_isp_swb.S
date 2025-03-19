/* *****************************************************
void wb_scale(
    int8_t px[32],
    const int8_t vk[32], 
    const int8_t vc[32],
    const int16_t vsat[16]
);

Descr:

    This allows to perform a int8 scale transormation.
    Even if the input array is 32 elements long, only the first 16 is used.

    Target: y_uint8 = factor * x_uint8 
    Translated in int8: y_int8 = factor * (x_int8 + 128) - 128
    Regrouping: y_int8 = factor * x_int8 + factor * (128 - 128/factor)
    Other: vc is preshifted left and saturated to match scale factor

    Then renaming:
        - vk = (128 - 128/factor)
        - vc = factor << sat
    Then: y_int8 = (vc * x_int8 + vc * vk ) >> sat

Op:
    px[0] = (vc[0] * px[0] + vc[0] * vk[0]) >> sat[0];

********************************************************/

#define FUNCTION_NAME   wb_scale
#define NSTACKWORDS     (0)             // 0 words of stack as the input register is used as output

#define px              r0
#define vk              r1
#define vc              r2
#define sat             r3

#define _12             r11
#define msk             r11

.text
.issue_mode dual
.align 4
.globl FUNCTION_NAME
.type FUNCTION_NAME, @function
.cc_top FUNCTION_NAME.func, FUNCTION_NAME

FUNCTION_NAME:
    dualentsp   NSTACKWORDS
    vclrdr
    ldc     _12, 12
    mkmsk   msk, _12
    vldc    vc[0]
    vlmacc  px[0]
    vlmacc  vk[0]
    vlsat   sat[0]   
    vstrpv  px[0], msk           
    retsp   NSTACKWORDS     

.cc_bottom FUNCTION_NAME.func
.set    FUNCTION_NAME.nstackwords, NSTACKWORDS; .globl  FUNCTION_NAME.nstackwords
.set    FUNCTION_NAME.maxcores,1; .global FUNCTION_NAME.maxcores
.set	FUNCTION_NAME.maxtimers,0; .globl FUNCTION_NAME.maxtimers
.set	FUNCTION_NAME.maxchanends,0; .globl	FUNCTION_NAME.maxchanends
.size FUNCTION_NAME, .-FUNCTION_NAME

#undef px
#undef vk
#undef vc
#undef sat
#undef _12
#undef msk
#undef FUNCTION_NAME
#undef NSTACKWORDS


/* *****************************************************
void wb_scale_image(
    int8_t *img,
    unsigned img_size,
    const int8_t vk[32], 
    const int8_t vc[32],
    const int16_t vsat[16]
);

Descr:

    Performs same operation as wb_scale but on a whole image.
    Note: the last 32 elements are not processed.

Op:
    for i in range(0, img_size - 32, 12):
        px[i] = (vc[0] * px[i] + vc[0] * vk[0]) >> sat[0];
    end for

********************************************************/

#define FUNCTION_NAME   wb_scale_image
#define NSTACKWORDS     (4)             // 0 words of stack as the input register is used as output
#define STACK_VSAT      (NSTACKWORDS + 1)

#define img             r0
#define img_size        r1
#define vk              r2
#define vc              r3

#define vsat            r4
#define _12             r5      
#define _32             r6
#define cnt             r7

#define msk             r11

.text
.issue_mode single
.align 4
.globl FUNCTION_NAME
.type FUNCTION_NAME, @function
.cc_top FUNCTION_NAME.func, FUNCTION_NAME

FUNCTION_NAME:
    ENTSP_lu6   NSTACKWORDS

// Save registers
    std r4, r5, sp[0]
    std r6, r7, sp[1]
    ldw vsat, sp[STACK_VSAT]
// Set Vpu
    vclrdr
    ldc     r11, 0x200
    vsetc   r11
// Constants
    ldc     _32, 32
    ldc     _12, 12
    mkmsk   msk, _12
    sub img_size, img_size, _32
    divu cnt, img_size, _12
    vldc    vc[0]
.do_wb_loop:
    vlmacc  img[0]
    vlmacc  vk[0]
    vlsat   vsat[0]   
    vstrpv  img[0], msk
    vclrdr
    add img, img, _12
    sub cnt, cnt, 1
    bt cnt, .do_wb_loop
// Restore regs
    ldd   r4, r5, sp[0]
    ldd   r6, r7, sp[1]
    retsp   NSTACKWORDS     

.cc_bottom FUNCTION_NAME.func
.set    FUNCTION_NAME.nstackwords, NSTACKWORDS; .globl  FUNCTION_NAME.nstackwords
.set    FUNCTION_NAME.maxcores,1; .global FUNCTION_NAME.maxcores
.set	FUNCTION_NAME.maxtimers,0; .globl FUNCTION_NAME.maxtimers
.set	FUNCTION_NAME.maxchanends,0; .globl	FUNCTION_NAME.maxchanends
.size FUNCTION_NAME, .-FUNCTION_NAME

#undef img
#undef img_size
#undef vk
#undef vc
#undef vsat
#undef _12
#undef _32
#undef rem
#undef msk
#undef STACK_VSAT
#undef FUNCTION_NAME
#undef NSTACKWORDS
