# REQUIRES: x86
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t
# RUN: ld.lld %t -o %t2
# RUN: llvm-objdump -t -section-headers %t2 | FileCheck %s

# CHECK: Sections:
# CHECK: Idx Name          Size     VMA          Type
# CHECK:   2 .bss          00000004 0000000000201000 BSS
# CHECK: SYMBOL TABLE:
# CHECK: 0000000000201000  .bss 00000000 __bss_start

# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi=purecap -mcpu=cheri128 %s -o %t
# RUN: ld.lld %t -o %t-cheri --verbose
# RUN: llvm-objdump -t -section-headers %t-cheri | FileCheck %s -check-prefix CHERI

# CHERI: Sections:
# CHERI: Idx Name          Size      VMA          Type
# CHERI:   6 .bss          00000004  0000000120010010 BSS
# CHERI: SYMBOL TABLE:
# CHERI: 0000000120010010  .bss 00000004 __bss_start

.global __bss_start
.text
_start:
.comm sym1,4,4
