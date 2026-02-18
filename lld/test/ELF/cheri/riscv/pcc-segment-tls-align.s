# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld %t.o -o %t
# RUN: llvm-readelf -l %t | FileCheck %s

## Verify the TLS segment's alignment offset is 0 even if in the middle of the
## RELRO segment.

# CHECK: TLS            0x000300 0x0000000000013300 0x0000000000013300 0x000001 0x000101 R   0x100

cllc ca0, .data.rel.ro

.data.rel.ro
.balign 2
.zero 1

.tdata
.zero 1

.tbss
.balign 256
.zero 1
