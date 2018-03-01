# REQUIRES: mips
# RUN: llvm-mc -triple=mips64-unknown-freebsd -filetype=obj %s -o %t.o
# RUN: ld.lld -shared %t.o -o %t.so
# RUN: llvm-readobj -expand-relocs -r -t %t.so

.global foo

.data
.global bar
bar:
  .quad foo
.size bar, 8


