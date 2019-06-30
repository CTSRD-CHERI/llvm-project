# REQUIRES: mips
# We should not be adding this symbol for MIPS output
# RUN: llvm-mc -position-independent -filetype=obj -triple=mips64-none-freebsd %s -o %t-mips.o
# RUN: not ld.lld --process-cap-relocs %t-mips.o -o %t.exe 2>&1 | FileCheck -check-prefix ERR-MIPS %s
# ERR-MIPS: error: undefined symbol: _CHERI_CAPABILITY_TABLE_

# But for CHERI purecap output it should exist even if the table is empty
# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld --process-cap-relocs %t.o -o %t.exe
# RUN: llvm-objdump -h -t %t.exe | FileCheck %s
#
# This used to cause LLD to create a symbol table with invalid entries
# Since the __cap_relocs section is empty it does not get added to the output
# file so the __start and __stop symbols reference an invalid section

.text
.global __start
__start:
  .option pic0
  dla $a0, _CHERI_CAPABILITY_TABLE_

# CHECK-LABEL: Sections:
# CHECK-NOT: __cap_relocs
# CHECK-NOT: .captable
# CHECK-LABEL: SYMBOL TABLE:
# CHECK-NEXT: 0000000000000000         *ABS*           00000000 .hidden _CHERI_CAPABILITY_TABLE_
# CHECK-NEXT: 0000000120027ff0         .got            00000000 .hidden _gp
# CHECK-NEXT: 0000000120010000         .text           00000000 __start
