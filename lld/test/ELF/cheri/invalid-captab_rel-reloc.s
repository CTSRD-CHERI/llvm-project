# REQUIRES: mips
# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -pie %t.o -o %t.exe
# RUsN: llvm-readobj --cap-relocs --dynamic-table %t.exe | FileCheck %s
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=EMPTY_CAP_TABLE=1 %s -o %t2.o
# This used to crash:
# RUN: not ld.lld -pie %t2.o -o %t.exe 2>&1 | FileCheck %s -check-prefix=EMPTY-TABLE

// EMPTY-TABLE: ld.lld: error: cannot compute difference between non-existent CheriCapabilityTable and symbol foo
// EMPTY-TABLE: ld.lld: error: cannot compute difference between non-existent CheriCapabilityTable and symbol foo


.text


# CHECK-LABEL: Disassembly of section .text:
.global __start
.ent __start
__start:
# CHECK-LABEL: __start:
.set noat
.set noreorder
.ifdef _USE_CHERI_CAPABILITY_TABLE_
# referencing _CHERI_CAPABILITY_TABLE_ fixes the crash:
lui        $1, %hi(%neg(%captab_rel(_CHERI_CAPABILITY_TABLE_)))
daddiu     $1, $1, %lo(%neg(%captab_rel(_CHERI_CAPABILITY_TABLE_)))
.endif
lui        $1, %hi(%neg(%captab_rel(foo)))
daddiu     $1, $1, %lo(%neg(%captab_rel(foo)))
nop

# Just to ensure we have something in the captable:
.ifdef EMPTY_CAP_TABLE
nop
.else
clcbi      $c1, %captab20(bar)($c1)
.endif
cjr        $c17
cld        $2, $zero, 0($c1)


.end __start

.data
.space 32
foo:
.8byte 9
.size foo, 8
bar:
.8byte 10
.size bar, 8
