//===-- save.S - save up to 12 callee-saved registers ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Multiple entry points depending on number of registers to save
//
//===----------------------------------------------------------------------===//

// The entry points are grouped up into 2s for rv64 and 4s for rv32 since this
// is the minimum grouping which will maintain the required 16-byte stack
// alignment.

  .text

#if __riscv_xlen == 32

  .globl  __riscv_save_12
  .type   __riscv_save_12,@function
__riscv_save_12:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset   csp, csp, -128
  li           t1, 0
  csc          cs11, 24(csp)
#else
  addi   sp, sp, -64
  mv     t1, zero
  sw     s11, 12(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_11_8

  .globl  __riscv_save_11
  .type   __riscv_save_11,@function
  .globl  __riscv_save_10
  .type   __riscv_save_10,@function
  .globl  __riscv_save_9
  .type   __riscv_save_9,@function
  .globl  __riscv_save_8
  .type   __riscv_save_8,@function
__riscv_save_11:
__riscv_save_10:
__riscv_save_9:
__riscv_save_8:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -128
  li         t1, 32
#else
  addi   sp, sp, -64
  li     t1, 16
#endif /* __CHERI_PURE_CAPABILITY__ */
.Lriscv_save_11_8:
#ifdef __CHERI_PURE_CAPABILITY__
  csc           cs10, 32(csp)
  csc           cs9,  40(csp)
  csc           cs8,  48(csp)
  csc           cs7,  56(csp)
#else
  sw     s10, 16(sp)
  sw     s9,  20(sp)
  sw     s8,  24(sp)
  sw     s7,  28(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_7_4

  .globl  __riscv_save_7
  .type   __riscv_save_7,@function
  .globl  __riscv_save_6
  .type   __riscv_save_6,@function
  .globl  __riscv_save_5
  .type   __riscv_save_5,@function
  .globl  __riscv_save_4
  .type   __riscv_save_4,@function
__riscv_save_7:
__riscv_save_6:
__riscv_save_5:
__riscv_save_4:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -128
  li         t1, 64
#else
  addi   sp, sp, -64
  li     t1, 32
#endif
.Lriscv_save_7_4:
#ifdef __CHERI_PURE_CAPABILITY__
  csc           cs6, 64(csp)
  csc           cs5, 72(csp)
  csc           cs4, 80(csp)
  csc           cs3, 88(csp)
  csc           cs2, 96(csp)
  csc           cs1, 104(csp)
  csc           cs0, 112(csp)
  csc           cra, 120(csp)
  cincoffset    csp, csp, t1
  cjr           ct0
#else
  sw     s6, 32(sp)
  sw     s5, 36(sp)
  sw     s4, 40(sp)
  sw     s3, 44(sp)
  sw     s2, 48(sp)
  sw     s1, 52(sp)
  sw     s0, 56(sp)
  sw     ra, 60(sp)
  add    sp, sp, t1
  jr     t0
#endif /* __CHERI_PURE_CAPABILITY__ */

  .globl  __riscv_save_3
  .type   __riscv_save_3,@function
  .globl  __riscv_save_2
  .type   __riscv_save_2,@function
  .globl  __riscv_save_1
  .type   __riscv_save_1,@function
  .globl  __riscv_save_0
  .type   __riscv_save_0,@function
__riscv_save_3:
__riscv_save_2:
__riscv_save_1:
__riscv_save_0:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -32
  csc        cs2,  0(csp)
  csc        cs1,  8(csp)
  csc        cs0,  16(csp)
  csc        cra,  24(csp)
  cjr        ct0
#else
  addi    sp, sp, -16
  sw      s2,  0(sp)
  sw      s1,  4(sp)
  sw      s0,  8(sp)
  sw      ra,  12(sp)
  jr      t0
#endif /* __CHERI_PURE_CAPABILITY__ */

#elif __riscv_xlen == 64

  .globl  __riscv_save_12
  .type   __riscv_save_12,@function
__riscv_save_12:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -224
  mv         t1, zero
  csc        cs11, 16(csp)
#else
  addi   sp, sp, -112
  mv     t1, zero
  sd     s11, 8(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_11_10

  .globl  __riscv_save_11
  .type   __riscv_save_11,@function
  .globl  __riscv_save_10
  .type   __riscv_save_10,@function
__riscv_save_11:
__riscv_save_10:
#ifdef __CHERI_PURE_CAPABILITY__
  addi   sp, sp, -224
  li     t1, 32
.Lriscv_save_11_10:
  csc    cs10, 32(csp)
  csc    cs9,  48(csp)
#else
  addi   sp, sp, -112
  li     t1, 16
.Lriscv_save_11_10:
  sd     s10, 16(sp)
  sd     s9,  24(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_9_8

  .globl  __riscv_save_9
  .type   __riscv_save_9,@function
  .globl  __riscv_save_8
  .type   __riscv_save_8,@function
__riscv_save_9:
__riscv_save_8:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -224
  li         t1, 64
.Lriscv_save_9_8:
  csc     cs8,  64(csp)
  csc     cs7,  80(csp)
#else
  addi   sp, sp, -112
  li     t1, 32
.Lriscv_save_9_8:
  sd     s8,  32(sp)
  sd     s7,  40(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_7_6

  .globl  __riscv_save_7
  .type   __riscv_save_7,@function
  .globl  __riscv_save_6
  .type   __riscv_save_6,@function
__riscv_save_7:
__riscv_save_6:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset csp, csp, -224
  li         t1, 96
.Lriscv_save_7_6:
  csc     cs6,  96(csp)
  csc     cs5,  112(csp)
#else
  addi   sp, sp, -112
  li     t1, 48
.Lriscv_save_7_6:
  sd     s6,  48(sp)
  sd     s5,  56(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_5_4

  .globl  __riscv_save_5
  .type   __riscv_save_5,@function
  .globl  __riscv_save_4
  .type   __riscv_save_4,@function
__riscv_save_5:
__riscv_save_4:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset   csp, csp, -224
  li           t1, 128
.Lriscv_save_5_4:
  csc     cs4, 128(csp)
  csc     cs3, 144(csp)
#else
  addi   sp, sp, -112
  li     t1, 64
.Lriscv_save_5_4:
  sd     s4, 64(sp)
  sd     s3, 72(sp)
#endif /* __CHERI_PURE_CAPABILITY__ */
  j      .Lriscv_save_3_2

  .globl  __riscv_save_3
  .type   __riscv_save_3,@function
  .globl  __riscv_save_2
  .type   __riscv_save_2,@function
__riscv_save_3:
__riscv_save_2:
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset   csp, csp, -224
  li           t1, 160
.Lriscv_save_3_2:
  csc            cs2, 160(csp)
  csc            cs1, 176(csp)
  csc            cs0, 192(csp)
  csc            cra, 208(csp)
  cincoffset     csp, csp, t1
  cjr            ct0
#else
  addi   sp, sp, -112
  li     t1, 80
.Lriscv_save_3_2:
  sd     s2, 80(sp)
  sd     s1, 88(sp)
  sd     s0, 96(sp)
  sd     ra, 104(sp)
  add    sp, sp, t1
  jr     t0
#endif /* __CHERI_PURE_CAPABILITY__ */

  .globl  __riscv_save_1
  .type   __riscv_save_1,@function
  .globl  __riscv_save_0
  .type   __riscv_save_0,@function
#ifdef __CHERI_PURE_CAPABILITY__
  cincoffset   csp, csp, -32
  csc          cs0, 0(csp)
  csc          cra, 16(csp)
  cjr          ct0
#else
  addi   sp, sp, -16
  sd     s0, 0(sp)
  sd     ra, 8(sp)
  jr     t0
#endif /* __CHERI_PURE_CAPABILITY__ */

#else
# error "xlen must be 32 or 64 for save-restore implementation
#endif
