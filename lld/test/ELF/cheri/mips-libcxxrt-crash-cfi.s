## This used to crash ld.lld for CHERI MIPS with "internal linker error: wrote incorrect addend value 0x1043000000000 instead of 0x10430"
## The crash no longer occurs after b2559f2f5ca950e917253b156bfd08ede5577cd5
# REQUIRES: mips
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd --position-independent %s -o %t.o
# RUN: ld.lld --eh-frame-hdr --shared -z notext -o %t.so %t.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-nopic.o
# RUN: ld.lld --eh-frame-hdr --shared -z notext -o %t.so %t-nopic.o
# RUN: llvm-readobj --hex-dump=.eh_frame %t.so | FileCheck %s

# CHECK-LABEL: Hex dump of section '.eh_frame':
# CHECK-NEXT: 0x000003f0 00000010 00000000 017a5200 01781f01 .........zR..x..
# CHECK-NEXT: 0x00000400 0c0d1d00 00000018 00000018 00000000 ................
# CHECK-NEXT: 0x00000410 00010430 00000000 00000004 00000000 ...0............
#                           ^ expected addend of 0x10430
# CHECK-NEXT: 0x00000420 00000000                            ....

.text
a:
.cfi_startproc
nop
.cfi_endproc
