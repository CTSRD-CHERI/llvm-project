# REQUIRES: mips
# RUN: llvm-mc -position-independent -filetype=obj -triple=mips64-none-freebsd %s -o %t.o
# RUN: not ld.lld %t.o -o /dev/null 2>&1 | FileCheck %s --check-prefix=MIPS
# These symbols are not automatically added for non-purecapL
# MIPS: error: undefined symbol: __start___cap_relocs
# MIPS: error: undefined symbol: __stop___cap_relocs
# RUN: %cheri128_llvm-mc -position-independent -filetype=obj -target-abi purecap %s -o %t.o
# RUN: ld.lld %t.o -o - | llvm-objdump -h -t - | FileCheck %s

#
# This used to cause LLD to create a symbol table with invalid entries
# Since the __cap_relocs section is empty it does not get added to the output
# file so the __start and __stop symbols reference an invalid section

.text
.global __start
.option pic0
__start:
  dla $a0, __start___cap_relocs
  dla $a1, __stop___cap_relocs
  
# CHECK-LABEL: Sections:
# CHECK-NOT: __cap_relocs
# CHECK-LABEL: SYMBOL TABLE:
# CHECK-NEXT: 0{{[0-9a-f]+}} l       .got		0000000000000000 .hidden _gp
# CHECK-NEXT: 0{{[0-9a-f]+}} g       .text	0000000000000000 __start
# CHECK-NEXT: 0000000000000000 g       *ABS*	0000000000000000 .protected __start___cap_relocs
# CHECK-NEXT: 0000000000000000 g       *ABS*	0000000000000000 .protected __stop___cap_relocs
