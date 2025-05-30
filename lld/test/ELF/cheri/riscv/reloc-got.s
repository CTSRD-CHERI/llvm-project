# REQUIRES: riscv
# RUN: echo '.globl b; b:' | %riscv32_cheri_purecap_llvm-mc -filetype=obj - -o %t1.o
# RUN: ld.lld -shared %t1.o -soname=t1.so -o %t1.so

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj -position-independent %s -o %t.o
# RUN: ld.lld %t.o %t1.so -o %t
# RUN: llvm-readelf -S %t | FileCheck --check-prefix=SEC32 %s
# RUN: llvm-readobj -r --cap-relocs %t | FileCheck --check-prefix=RELOC32 %s
# RUN: llvm-nm %t | FileCheck --check-prefix=NM32 %s
# RUN: llvm-readobj -x .got %t | FileCheck --check-prefix=HEX32 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t | FileCheck --check-prefix=DIS32 %s

# RUN: echo '.globl b; b:' | %riscv64_cheri_purecap_llvm-mc -filetype=obj - -o %t1.o
# RUN: ld.lld -shared %t1.o -soname=t1.so -o %t1.so

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj -position-independent %s -o %t.o
# RUN: ld.lld %t.o %t1.so -o %t
# RUN: llvm-readelf -S %t | FileCheck --check-prefix=SEC64 %s
# RUN: llvm-readobj -r --cap-relocs %t | FileCheck --check-prefix=RELOC64 %s
# RUN: llvm-nm %t | FileCheck --check-prefix=NM64 %s
# RUN: llvm-readobj -x .got %t | FileCheck --check-prefix=HEX64 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t | FileCheck --check-prefix=DIS64 %s

# SEC32: .got PROGBITS         00012230 000230 000018
# SEC64: .got PROGBITS 00000000000123a0 0003a0 000030

# RELOC32:      .rela.dyn {
# RELOC32-NEXT:   0x12238 R_RISCV_CHERI_CAPABILITY b 0x0
# RELOC32-NEXT: }
# RELOC32:      CHERI __cap_relocs [
# RELOC32-NEXT:   0x012240 Base: 0x13248 (a+0) Length: 4 Perms: Object
# RELOC32-NEXT: ]

# RELOC64:      .rela.dyn {
# RELOC64-NEXT:   0x123B0 R_RISCV_CHERI_CAPABILITY b 0x0
# RELOC64-NEXT: }
# RELOC64:      CHERI __cap_relocs [
# RELOC64-NEXT:   0x0123c0 Base: 0x133d0 (a+0) Length: 4 Perms: Object
# RELOC64-NEXT: ]

# NM32: 00013248 d a
# NM64: 00000000000133d0 d a

## .got[0] = _DYNAMIC
## .got[1] = 0 (relocated by R_RISCV_CHERI_CAPABILITY at run time)
## .got[2] = 0 (relocated by __cap_relocs at run time)
# HEX32: section '.got':
# HEX32: 0x00012230 c0210100 00000000 00000000 00000000
# HEX32: 0x00012240 00000000 00000000

# HEX64: section '.got':
# HEX64: 0x000123a0 c0220100 00000000 00000000 00000000
# HEX64: 0x000123b0 00000000 00000000 00000000 00000000
# HEX64: 0x000123c0 00000000 00000000 00000000 00000000

## &.got[2]-. = 0x12240-0x111b0 = 4096*1+144
# DIS32:      111b0: auipcc ca0, 1
# DIS32-NEXT:        lc ca0, 144(ca0)
## &.got[1]-. = 0x12238-0x111b8 = 4096*1+128
# DIS32:      111b8: auipcc ca0, 1
# DIS32-NEXT:        lc ca0, 128(ca0)

## &.got[2]-. = 0x123c0-0x112b0 = 4096*1+272
# DIS64:      112b0: auipcc ca0, 1
# DIS64-NEXT:        lc ca0, 272(ca0)
## &.got[1]-. = 0x123b0-0x112b8 = 4096*1+248
# DIS64:      112b8: auipcc ca0, 1
# DIS64-NEXT:        lc ca0, 248(ca0)

clgc ca0, a
clgc ca0, b

.data
a:
## An undefined reference of _GLOBAL_OFFSET_TABLE_ causes .got[0] to be
## allocated to store _DYNAMIC.
.long _GLOBAL_OFFSET_TABLE_ - .
