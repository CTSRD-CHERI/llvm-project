# REQUIRES: riscv

# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o -o %t.32
# RUN: llvm-readobj --cap-relocs %t.32 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .data %t.32 | FileCheck --check-prefix=HEX32 %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o -o %t.64
# RUN: llvm-readobj --cap-relocs %t.64 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .data %t.64 | FileCheck --check-prefix=HEX64 %s

# RELOC: There is no __cap_relocs section in the file.

# HEX32-LABEL: section '.data':
# HEX32-NEXT:  [[#%x,]] 00000000 00000000 03000000 00000000

# HEX64-LABEL: section '.data':
# HEX64-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX64-NEXT:  [[#%x,]] 03000000 00000000 00000000 00000000

.weak undef_weak

.data
.chericap undef_weak
.chericap undef_weak + 3
