/* *****************************************************
void raw8_block_to_yuv422(
    int8_t *dst,
    int8_t *src,
    unsigned line_size
);

********************************************************/

#define FUNCTION_NAME   raw8_block_to_yuv422
#define NSTACKWORDS     2
#define dst             r0
#define src             r1
#define lns             r2 // line size
#define msk             r3

#define _16             r4      
#define _32             r5

.section .dp.rodata,"awd",@progbits
.issue_mode dual
.align 4
cp_xor: .word 0x80808080

.section .text
.issue_mode dual
.align 4
.globl FUNCTION_NAME
.type FUNCTION_NAME, @function
.cc_top FUNCTION_NAME.func, FUNCTION_NAME

FUNCTION_NAME:
    DUALENTSP_lu6   NSTACKWORDS
    // init
    ldc r11, 0x200
    std r4, r5, sp[0]
    {ldc _32, 32; ldc _16, 16}
    {vclrdr; nop}
    {mkmsk msk, _16; nop}
    {vsetc r11; mov r11, src} 
    // make block 2x16
    {vldr r11[0]} //TODO >> from here all starts to fail
    vstrpv dst[0], msk
    {add dst, dst, _16; add r11, r11, lns}
    {vldr r11[0]; nop}
    vstrpv dst[0], msk
    {vclrdr; sub dst, dst, _16}
    {vldc dst[0]; nop}
    // loop
    ldaw r11, cp[kernels_yuv422]
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    {vlmaccr  r11[0]; add r11, r11, _32}
    ldaw r11, cp[kernels_yuv422_sat]
    {vlsat r11[0] ; nop}  // saturation
    ldaw r11, cp[kernels_yuv422_adds]
    {vladd r11[0] ; nop} // add
    {vstr dst[0] ; nop}  // result
    // do xors [0:8]
    ldw r11, dp[cp_xor]
    ldd r4, r5, dst[0]
    xor r4, r4, r11
    xor r5, r5, r11
    std r4, r5, dst[0]
    // do xors [8:16]
    ldd r4, r5, dst[1]
    xor r4, r4, r11
    xor r5, r5, r11
    std r4, r5, dst[1]
    // restore regs
    ldd  r4, r5, sp[0]
    RETSP_lu6 NSTACKWORDS

.cc_bottom FUNCTION_NAME.func
.set    FUNCTION_NAME.nstackwords, NSTACKWORDS; .globl  FUNCTION_NAME.nstackwords
.set    FUNCTION_NAME.maxcores,1; .global FUNCTION_NAME.maxcores
.set	FUNCTION_NAME.maxtimers,0; .globl FUNCTION_NAME.maxtimers
.set	FUNCTION_NAME.maxchanends,0; .globl	FUNCTION_NAME.maxchanends
.size FUNCTION_NAME, .-FUNCTION_NAME
