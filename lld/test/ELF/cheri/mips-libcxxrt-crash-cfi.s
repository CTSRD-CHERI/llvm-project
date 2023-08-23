## This used to crash ld.lld for CHERI MIPS
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd --position-independent %s -o %t.o
# RUN: ld.lld --eh-frame-hdr --shared -z notext -o %t.so %t.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-nopic.o
# RUN: not ld.lld --eh-frame-hdr --shared -z notext -o %t.so %t-nopic.o 2>&1 | FileCheck %s --check-prefix=ERR
# ERR: internal linker error: wrote incorrect addend value 0x1043000000000 instead of 0x10430 for dynamic relocation R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE

.text
a:
.cfi_startproc
nop
.cfi_endproc
