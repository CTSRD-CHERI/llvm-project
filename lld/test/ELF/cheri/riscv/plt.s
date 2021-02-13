# REQUIRES: riscv
# RUN: echo '.globl bar, weak; .type bar,@function; .type weak,@function; bar: weak:' > %t1.s

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %t1.s -o %t1.32.o
# RUN: ld.lld -shared %t1.32.o -soname=t1.32.so -o %t1.32.so
# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o %t1.32.so -z separate-code -o %t.32
# RUN: llvm-readelf -S -s %t.32 | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r %t.32 | FileCheck --check-prefix=RELOC32 %s
# RUN: llvm-readelf -x .captable %t.32 | FileCheck --check-prefix=CAPTAB32 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32 | FileCheck --check-prefixes=DIS,DIS32 %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t1.s -o %t1.64.o
# RUN: ld.lld -shared %t1.64.o -soname=t1.64.so -o %t1.64.so
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o %t1.64.so -z separate-code -o %t.64
# RUN: llvm-readelf -S -s %t.64 | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r %t.64 | FileCheck --check-prefix=RELOC64 %s
# RUN: llvm-readelf -x .captable %t.64 | FileCheck --check-prefix=CAPTAB64 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64 | FileCheck --check-prefixes=DIS,DIS64 %s

# SEC: .plt PROGBITS {{0*}}00011030

## A canonical PLT has a non-zero st_value. bar and weak are called but their
## addresses are not taken, so a canonical PLT is not necessary.
# NM: {{0*}}00000000 0 FUNC GLOBAL DEFAULT UND bar
# NM: {{0*}}00000000 0 FUNC WEAK   DEFAULT UND weak

# RELOC32:      .rela.dyn {
# RELOC32-NEXT:   0x12000 R_RISCV_CHERI_CAPABILITY bar 0x0
# RELOC32-NEXT:   0x12008 R_RISCV_CHERI_CAPABILITY weak 0x0
# RELOC32-NEXT: }
# CAPTAB32:      section '.captable'
# CAPTAB32-NEXT: 0x00012000 00000000 00000000 00000000 00000000

# RELOC64:      .rela.dyn {
# RELOC64-NEXT:   0x12000 R_RISCV_CHERI_CAPABILITY bar 0x0
# RELOC64-NEXT:   0x12010 R_RISCV_CHERI_CAPABILITY weak 0x0
# RELOC64-NEXT: }
# CAPTAB64:      section '.captable'
# CAPTAB64-NEXT: 0x00012000 00000000 00000000 00000000 00000000
# CAPTAB64-NEXT: 0x00012010 00000000 00000000 00000000 00000000

# DIS:      <_start>:
## Direct call
## foo - . = 0x11020-0x11000 = 32
# DIS-NEXT:   11000: auipcc cra, 0
# DIS-NEXT:          cjalr 32(cra)
## bar@plt - . = 0x11050-0x11008 = 72
# DIS-NEXT:   11008: auipcc cra, 0
# DIS-NEXT:          cjalr 72(cra)
## bar@plt - . = 0x11050-0x11010 = 64
# DIS-NEXT:   11010: auipcc cra, 0
# DIS-NEXT:          cjalr 64(cra)
## weak@plt - . = 0x11060-0x11018 = 72
# DIS-NEXT:   11018: auipcc cra, 0
# DIS-NEXT:          cjalr 72(cra)
# DIS:      <foo>:
# DIS-NEXT:   11020:

# DIS:      Disassembly of section .plt:
# DIS:      <.plt>:
# DIS-NEXT:     ...

## 32-bit: &.captable[bar]-. = 0x12000-0x11050 = 4096*1-80
# DIS:        11050: auipcc ct3, 1
# DIS32-NEXT:   clc ct3, -80(ct3)
# DIS64-NEXT:   clc ct3, -80(ct3)
# DIS-NEXT:     cjalr ct1, ct3
# DIS-NEXT:     nop

## 32-bit: &.captable[weak]-. = 0x12008-0x11060 = 4096*1-88
# DIS:        11060: auipcc ct3, 1
# DIS32-NEXT:   clc ct3, -88(ct3)
# DIS64-NEXT:   clc ct3, -80(ct3)
# DIS-NEXT:     cjalr ct1, ct3
# DIS-NEXT:     nop

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
