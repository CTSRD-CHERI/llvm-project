# REQUIRES: riscv
# RUN: echo '.globl bar, weak; .type bar,@function; .type weak,@function; bar: weak:' > %t1.s

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %t1.s -o %t1.32.o
# RUN: ld.lld -shared %t1.32.o -soname=t1.32.so -o %t1.32.so
# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o %t1.32.so -z separate-code -o %t.32.got
# RUN: llvm-readelf -S -s %t.32.got | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r --cap-relocs %t.32.got | FileCheck --check-prefix=RELOCGOT32 %s
# RUN: llvm-readelf -x .got %t.32.got | FileCheck --check-prefix=GOT32 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32.got | FileCheck --check-prefixes=DIS,DISGOT,DISGOT32 %s
# RUN: ld.lld %t.32.o %t1.32.so -z separate-code -z cheri-riscv-jump-slot -o %t.32.got.plt
# RUN: llvm-readelf -S -s %t.32.got.plt | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r --cap-relocs %t.32.got.plt | FileCheck --check-prefix=RELOCGOTPLT32 %s
# RUN: llvm-readelf -x .got.plt %t.32.got.plt | FileCheck --check-prefix=GOTPLT32 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32.got.plt | FileCheck --check-prefixes=DIS,DISGOTPLT,DISGOTPLT32 %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t1.s -o %t1.64.o
# RUN: ld.lld -shared %t1.64.o -soname=t1.64.so -o %t1.64.so
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o %t1.64.so -z separate-code -o %t.64.got
# RUN: llvm-readelf -S -s %t.64.got | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r --cap-relocs %t.64.got | FileCheck --check-prefix=RELOCGOT64 %s
# RUN: llvm-readelf -x .got %t.64.got | FileCheck --check-prefix=GOT64 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64.got | FileCheck --check-prefixes=DIS,DISGOT,DISGOT64 %s
# RUN: ld.lld %t.64.o %t1.64.so -z separate-code -z cheri-riscv-jump-slot -o %t.64.got.plt
# RUN: llvm-readelf -S -s %t.64.got.plt | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r --cap-relocs %t.64.got.plt | FileCheck --check-prefix=RELOCGOTPLT64 %s
# RUN: llvm-readelf -x .got.plt %t.64.got.plt | FileCheck --check-prefix=GOTPLT64 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64.got.plt | FileCheck --check-prefixes=DIS,DISGOTPLT,DISGOTPLT64 %s

# SEC: .plt PROGBITS {{0*}}00011030

## A canonical PLT has a non-zero st_value. bar and weak are called but their
## addresses are not taken, so a canonical PLT is not necessary.
# NM: {{0*}}00000000 0 FUNC GLOBAL DEFAULT UND bar
# NM: {{0*}}00000000 0 FUNC WEAK   DEFAULT UND weak

# RELOCGOT32:      .rela.dyn {
# RELOCGOT32-NEXT:   0x12068 R_RISCV_CHERI_CAPABILITY bar 0x0
# RELOCGOT32-NEXT:   0x12070 R_RISCV_CHERI_CAPABILITY weak 0x0
# RELOCGOT32-NEXT: }
# RELOCGOT32:      There is no __cap_relocs section in the file.
# GOT32:      section '.got'
# GOT32-NEXT: 0x00012060 00200100 00000000 00000000 00000000
# GOT32-NEXT: 0x00012070 00000000 00000000

# RELOCGOTPLT32:      .rela.plt {
# RELOCGOTPLT32-NEXT:   0x13088 R_RISCV_JUMP_SLOT bar 0x0
# RELOCGOTPLT32-NEXT:   0x13090 R_RISCV_JUMP_SLOT weak 0x0
# RELOCGOTPLT32-NEXT: }
# RELOCGOTPLT32:      CHERI __cap_relocs [
# RELOCGOTPLT32-NEXT:   0x013088 Base: 0x11030 (<unknown symbol>+0) Length: 64 Perms: Function
# RELOCGOTPLT32-NEXT:   0x013090 Base: 0x11030 (<unknown symbol>+0) Length: 64 Perms: Function
# RELOCGOTPLT32-NEXT: ]
# GOTPLT32:      section '.got.plt'
# GOTPLT32-NEXT: 0x00013078 00000000 00000000 00000000 00000000
# GOTPLT32-NEXT: 0x00013088 00000000 00000000 00000000 00000000

# RELOCGOT64:      .rela.dyn {
# RELOCGOT64-NEXT:   0x120D0 R_RISCV_CHERI_CAPABILITY bar 0x0
# RELOCGOT64-NEXT:   0x120E0 R_RISCV_CHERI_CAPABILITY weak 0x0
# RELOCGOT64-NEXT: }
# RELOCGOT64:      There is no __cap_relocs section in the file.
# GOT64:      section '.got'
# GOT64-NEXT: 0x000120c0 00200100 00000000 00000000 00000000
# GOT64-NEXT: 0x000120d0 00000000 00000000 00000000 00000000
# GOT64-NEXT: 0x000120e0 00000000 00000000 00000000 00000000

# RELOCGOTPLT64:      .rela.plt {
# RELOCGOTPLT64-NEXT:   0x13110 R_RISCV_JUMP_SLOT bar 0x0
# RELOCGOTPLT64-NEXT:   0x13120 R_RISCV_JUMP_SLOT weak 0x0
# RELOCGOTPLT64-NEXT: }
# RELOCGOTPLT64:      CHERI __cap_relocs [
# RELOCGOTPLT64-NEXT:   0x013110 Base: 0x11030 (<unknown symbol>+0) Length: 64 Perms: Function
# RELOCGOTPLT64-NEXT:   0x013120 Base: 0x11030 (<unknown symbol>+0) Length: 64 Perms: Function
# RELOCGOTPLT64-NEXT: ]
# GOTPLT64:      section '.got.plt'
# GOTPLT64-NEXT: 0x000130f0 00000000 00000000 00000000 00000000
# GOTPLT64-NEXT: 0x00013100 00000000 00000000 00000000 00000000
# GOTPLT64-NEXT: 0x00013110 00000000 00000000 00000000 00000000
# GOTPLT64-NEXT: 0x00013120 00000000 00000000 00000000 00000000

# DIS:      <_start>:
## Direct call
## foo - . = 0x11020-0x11000 = 32
# DIS-NEXT:   11000: auipcc cra, 0
# DIS-NEXT:          jalr 32(cra)
## bar@plt - . = 0x11050-0x11008 = 72
# DIS-NEXT:   11008: auipcc cra, 0
# DIS-NEXT:          jalr 72(cra)
## bar@plt - . = 0x11050-0x11010 = 64
# DIS-NEXT:   11010: auipcc cra, 0
# DIS-NEXT:          jalr 64(cra)
## weak@plt - . = 0x11060-0x11018 = 72
# DIS-NEXT:   11018: auipcc cra, 0
# DIS-NEXT:          jalr 72(cra)
# DIS:      <foo>:
# DIS-NEXT:   11020:

# DIS:              Disassembly of section .plt:
# DIS:              <.plt>:
# DISGOT-NEXT:          ...
# DISGOTPLT-NEXT:       auipcc ct2, 2
# DISGOTPLT-NEXT:       sub t1, t1, t3
## 32-bit: .got.plt - .plt = 0x13078 - 0x11030 = 4096*2+72
## 64-bit: .got.plt - .plt = 0x130f0 - 0x11030 = 4096*2+192
# DISGOTPLT32-NEXT:     lc ct3, 72(ct2)
# DISGOTPLT64-NEXT:     lc ct3, 192(ct2)
# DISGOTPLT-NEXT:       addi t1, t1, -44
# DISGOTPLT32-NEXT:     cincoffset ct0, ct2, 72
# DISGOTPLT64-NEXT:     cincoffset ct0, ct2, 192
# DISGOTPLT32-NEXT:     srli t1, t1, 1
# DISGOTPLT32-NEXT:     lc ct0, 8(ct0)
# DISGOTPLT64-NEXT:     lc ct0, 16(ct0)
# DISGOTPLT-NEXT:       jr ct3
# DISGOTPLT64-NEXT:     nop

## 32-bit (.got): &.got[bar]-. = 0x12068-0x11050 = 4096*1+24
## 64-bit (.got): &.got[bar]-. = 0x120d0-0x11050 = 4096*1+128
## 32-bit (.got.plt): &.got.plt[bar]-. = 0x13088-0x11050 = 4096*2+56
## 64-bit (.got.plt): &.got.plt[bar]-. = 0x13110-0x11050 = 4096*2+192
# DISGOT:           11050: auipcc ct3, 1
# DISGOTPLT:        11050: auipcc ct3, 2
# DISGOT32-NEXT:      lc ct3, 24(ct3)
# DISGOT64-NEXT:      lc ct3, 128(ct3)
# DISGOTPLT32-NEXT:   lc ct3, 56(ct3)
# DISGOTPLT64-NEXT:   lc ct3, 192(ct3)
# DIS-NEXT:           jalr ct1, ct3
# DIS-NEXT:           nop

## 32-bit (.got): &.got[weak]-. = 0x12070-0x11060 = 4096*1+16
## 64-bit (.got): &.got[weak]-. = 0x120e0-0x11060 = 4096*1+128
## 32-bit (.got.plt): &.got.plt[weak]-. = 0x13090-0x11060 = 4096*2+48
## 64-bit (.got.plt): &.got.plt[weak]-. = 0x13120-0x11060 = 4096*2+192
# DISGOT:           11060: auipcc ct3, 1
# DISGOTPLT:        11060: auipcc ct3, 2
# DISGOT32-NEXT:      lc ct3, 16(ct3)
# DISGOT64-NEXT:      lc ct3, 128(ct3)
# DISGOTPLT32-NEXT:   lc ct3, 48(ct3)
# DISGOTPLT64-NEXT:   lc ct3, 192(ct3)
# DIS-NEXT:           jalr ct1, ct3
# DIS-NEXT:           nop

.global _start, foo, bar
.weak weak

.type _start, @function
_start:
  ccall foo
  ccall bar
  ccall bar@plt
  ccall weak
.size _start, . - _start

## foo is local and non-preemptale, no PLT is generated.
.type foo, @function
foo:
  cret
.size foo, . - foo
