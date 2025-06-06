# RUN: llvm-mc --triple=riscv64-codasip-linux-musl -target-abi l64pc128 -mattr=+zcheripurecap,+zcherihybrid,+cap-mode %s -filetype=obj -o %t.o
# RUN: llvm-readelf -r %t.o | FileCheck %s -check-prefix OBJ
# RUN: ld.lld %t.o -pie -o %t.exe
# RUN: llvm-readelf -r %t.exe | FileCheck %s -check-prefix EXE


# OBJ:      Relocation section '.rela.text'
# OBJ:      Offset
# OBJ-NEXT: [[Y_ADDR:[0-9a-f]+]]     {{[0-9a-f]+}} R_RISCV_GOT_HI20 {{[0-9a-f]+}} y + 0
# OBJ-NEXT: [[START_ADDR:[0-9a-f]+]] {{[0-9a-f]+}} R_RISCV_PCREL_LO12_I            {{[0-9a-f]+}} _start + 0
# OBJ:      Relocation section '.rela.data.rel.ro'
# OBJ-NEXT: Offset
# OBJ:      [[Y_ADDR]] {{[0-9a-f]+}} R_RISCV_64 {{[0-9a-f]+}} x + 0


# EXE:      Relocation section '.rela.dyn'
# EXE-NEXT: Offset
# EXE-NEXT: [[REL_OFFSET:[0-9a-f]+]] {{[0-9a-f]+}} R_RISCV_RELATIVE

  .text
  .globl _start
  .type main,@function
_start:
  auipc ca1, %got_pcrel_hi(y)
  lc ca1, %pcrel_lo(_start)(ca1)
  ld a0, 0(ca1)
  ret
.size _start, .-_start

  .globl x
  .section .sdata,"aw",@progbits
  .type x,@object
x:
  .word 33
  .size x, 4

  .globl y
  .section .data.rel.ro,"aw",@progbits
  .type y,@object
y:
  .quad x
  .size y, 8
