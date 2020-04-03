# REQUIRES: mips
# RUN: llvm-mc -position-independent -filetype=obj -triple=mips64-none-freebsd %s -o %t.o
# RUN: ld.lld --process-cap-relocs %t.o -o %t.exe
# RUN: llvm-objdump -h -t %t.exe | FileCheck %s
#
# This used to cause LLD to create a symbol table with invalid entries
# Since the __cap_relocs section is empty it does not get added to the output
# file so the __start and __stop symbols reference an invalid section

.text
.global __start
__start:
  dla $a0, __start___cap_relocs
  dla $a1, __stop___cap_relocs
  
# CHECK-LABEL: Sections:
# CHECK-NOT: __cap_relocs
# CHECK-LABEL: SYMBOL TABLE:
# CHECK-NEXT: 0000000000038210 l       .got		0000000000000000 .hidden _gp
# CHECK-NEXT: 0000000000020210 g       .text	0000000000000000 __start
# CHECK-NEXT: 0000000000000000 g       *ABS*	0000000000000000 .protected __start___cap_relocs
# CHECK-NEXT: 0000000000000000 g       *ABS*	0000000000000000 .protected __stop___cap_relocs
