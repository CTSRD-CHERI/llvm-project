# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %t/abs.s -o %t/abs.32.o
# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %t/use.s -o %t/use.32.o
# RUN: ld.lld -T %t/lds --defsym abs3=0x42424242 %t/abs.32.o %t/use.32.o -o %t.32
# RUN: llvm-readobj --cap-relocs %t.32 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .got -x .data %t.32 | FileCheck --check-prefix=HEX32 %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/abs.s -o %t/abs.64.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/use.s -o %t/use.64.o
# RUN: ld.lld -T %t/lds --defsym abs3=0x42424242 %t/abs.64.o %t/use.64.o -o %t.64
# RUN: llvm-readobj --cap-relocs %t.64 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .got -x .data %t.64 | FileCheck --check-prefix=HEX64 %s

# RELOC-LABEL: CHERI Capability Relocations [
# RELOC-NEXT:  ]

# HEX32-LABEL: section '.got':
# HEX32-NEXT:  [[#%x,]] 00000000 00000000 78563412 00000000
# HEX32-NEXT:  [[#%x,]] b55aab00 00000000 42424242 00000000
# HEX32-LABEL: section '.data':
# HEX32-NEXT:  [[#%x,]] 78563412 00000000 7b563412 00000000
# HEX32-NEXT:  [[#%x,]] b55aab00 00000000 b95aab00 00000000
# HEX32-NEXT:  [[#%x,]] 42424242 00000000 47424242 00000000

# HEX64-LABEL: section '.got':
# HEX64-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 78563412 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] b55aab00 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 42424242 00000000 00000000 00000000
# HEX64-LABEL: section '.data':
# HEX64-NEXT:  [[#%x,]] 78563412 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 7b563412 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] b55aab00 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] b95aab00 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 42424242 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 47424242 00000000 00000000 00000000

#--- abs.s

.global abs
.type abs, %object
.set abs, 0x12345678
.size abs, 0xdead

#--- lds

abs2 = 0xab5ab5;

#--- use.s

clgc ca0, abs
clgc ca0, abs2
clgc ca0, abs3

.data
.chericap abs
.chericap abs + 3
.chericap abs2
.chericap abs2 + 4
.chericap abs3
.chericap abs3 + 5
