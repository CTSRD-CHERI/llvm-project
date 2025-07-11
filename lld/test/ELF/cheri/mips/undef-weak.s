# REQUIRES: mips

# RUN: llvm-mc -triple=mips64 -mcpu=cheri128 -mattr=+cheri128 -filetype=obj %s -o %t.128.o
# RUN: ld.lld %t.128.o -o %t.128
# RUN: llvm-readobj --cap-relocs %t.128 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .data %t.128 | FileCheck --check-prefix=HEX128 %s

# RUN: llvm-mc -triple=mips64 -mcpu=cheri256 -mattr=+cheri256 -filetype=obj %s -o %t.256.o
# RUN: ld.lld %t.256.o -o %t.256
# RUN: llvm-readobj --cap-relocs %t.256 | FileCheck --check-prefix=RELOC %s
# RUN: llvm-readobj -x .data %t.256 | FileCheck --check-prefix=HEX256 %s

# RELOC: There is no __cap_relocs section in the file.

# HEX128-LABEL: section '.data':
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX128-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000003

# HEX256-LABEL: section '.data':
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000003
# HEX256-NEXT:  [[#%x,]] 00000000 00000000 00000000 00000000

.weak undef_weak

.data
.chericap undef_weak
.chericap undef_weak + 3
