# Check R_MIPS_PCxxx relocations calculation.

# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %s -o %t1.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %S/Inputs/mips-dynamic.s -o %t2.o
# RUN: ld.lld %t1.o %t2.o -image-base=0x10000 -o %t.exe
# RUN: llvm-objdump -triple=mips64-unknown-freebsd -d -t -s %t.exe | FileCheck %s

# REQUIRES: mips

  .text
  .globl  __start
__start:
  lui       $2, %pcrel_hi(_foo - 4)     # R_MIPS_PCHI16
  daddiu    $2, $2, %pcrel_lo(_foo)     # R_MIPS_PCLO16
  nop
  # now against an address further back:
  lui       $2, %pcrel_hi(__start - 4)  # R_MIPS_PCHI16
  daddiu    $2, $2, %pcrel_lo(__start)  # R_MIPS_PCLO16
  nop

  .data

# CHECK:      Disassembly of section .text:
# CHECK-EMPTY:
# CHECK-NEXT: __start:
# CHECK-NEXT:    20000:       3c 02 00 00     lui     $2, 0
#                                      ^-- (0x20020-0x4-0x20000)>>2
# CHECK-NEXT:    20004:       64 42 00 1c     daddiu  $2, $2, 28
#                                      ^-- (0x20020-0x20004)>>2
# CHECK-NEXT:    20008:       00 00 00 00     nop
# CHECK-NEXT:    2000c:       3c 02 00 00 lui $2, 0
#                                      ^-- (0x20000-0x4-0x2000c))>>2
# CHECK-NEXT:    20010:       64 42 ff f0     daddiu  $2, $2, -16
#                                      ^-- (0x20000-0x2000c)>>2
# CHECK-NEXT:    20014:       00 00 00 00     nop
