# REQUIRES: mips
# RUN: split-file %s %t

# RUN: llvm-mc -triple=mips64 -mcpu=cheri128 -mattr=+cheri128 -filetype=obj %t/abs.s -o %t/abs.128.o
# RUN: llvm-mc -triple=mips64 -mcpu=cheri128 -mattr=+cheri128 -filetype=obj %t/use.s -o %t/use.128.o
# RUN: ld.lld -T %t/lds --defsym abs3=0x42424242 %t/abs.128.o %t/use.128.o -o %t.128
# RUN: llvm-readobj --cap-relocs %t.128 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .captable -x .data %t.128 | FileCheck --check-prefix=HEX128 %s

# RUN: llvm-mc -triple=mips64 -mcpu=cheri256 -mattr=+cheri256 -filetype=obj %t/abs.s -o %t/abs.256.o
# RUN: llvm-mc -triple=mips64 -mcpu=cheri256 -mattr=+cheri256 -filetype=obj %t/use.s -o %t/use.256.o
# RUN: ld.lld -T %t/lds --defsym abs3=0x42424242 %t/abs.256.o %t/use.256.o -o %t.256
# RUN: llvm-readobj --cap-relocs %t.256 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .captable -x .data %t.256 | FileCheck --check-prefix=HEX256 %s

# RELOC-LABEL: CHERI Capability Relocations [
# RELOC-NEXT:  ]

# HEX128-LABEL: section '.captable':
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 12345678
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab5
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424242
# HEX128-LABEL: section '.data':
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 12345678
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 1234567b
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab5
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab9
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424242
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424247

# HEX256-LABEL: section '.captable':
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 12345678
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab5
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424242
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-LABEL: section '.data':
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 12345678
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 1234567b
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab5
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00ab5ab9
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424242
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 42424247
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000

#--- abs.s

.global abs
.type abs, %object
.set abs, 0x12345678
.size abs, 0xdead

#--- lds

abs2 = 0xab5ab5;

#--- use.s

clcbi $c1, %captab20(abs)($c1)
clcbi $c1, %captab20(abs2)($c1)
clcbi $c1, %captab20(abs3)($c1)

.data
.chericap abs
.chericap abs + 3
.chericap abs2
.chericap abs2 + 4
.chericap abs3
.chericap abs3 + 5
