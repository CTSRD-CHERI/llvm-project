# REQUIRES: x86, mips
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t
# RUN: ld.lld %t -o %t2
# RUN: llvm-objdump -t --section-headers %t2 | FileCheck %s

## Test __bss_start is defined at the start of .bss

# CHECK: Sections:
# CHECK: Idx Name          Size     VMA                 Type
# CHECK:   2 .bss          00000004 [[ADDR:[0-za-f]+]]  BSS
# CHECK: SYMBOL TABLE:
# CHECK: [[ADDR]] g        .bss 0000000000000000 __bss_start

# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi=purecap -mcpu=cheri128 %s -o %t
# RUN: ld.lld %t -o %t-cheri --verbose
# RUN: llvm-objdump -t --section-headers %t-cheri | FileCheck %s -check-prefix CHERI

# CHERI: Sections:
# CHERI: Idx Name          Size      VMA          Type
# CHERI:   6 .bss          00000010  [[ADDR:[0-9a-f]+]] BSS
# CHERI: SYMBOL TABLE:
# CHERI: [[ADDR]] g        .bss 0000000000000010 __bss_start

.global __bss_start
.text
_start:
.comm sym1,4,4
