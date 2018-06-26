# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld %t.o -o %t.exe
# RUN: llvm-objdump -C -t -d -h %t.exe | FileCheck %s
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=EMPTY_CAP_TABLE=1 %s -o %t2.o
# RUN: ld.lld %t2.o -o %t.exe
# RUN: llvm-objdump -C -t -d -h %t.exe | FileCheck %s -check-prefix EMPTY-TABLE

.text


# CHECK-LABEL: Disassembly of section .text:

.global __start
.ent __start
__start:
# CHECK-LABEL: __start:
.set noat
.set noreorder
# TODO: check for overflow?

lui        $4, %highest(_CHERI_CAPABILITY_TABLE_)
lui        $3, %higher(_CHERI_CAPABILITY_TABLE_)
lui        $2, %hi(_CHERI_CAPABILITY_TABLE_)
lui        $1, %lo(_CHERI_CAPABILITY_TABLE_)
# Address is 0x40000:
# CHECK:      lui $4, 0
# CHECK-NEXT: lui $3, 0
# CHECK-NEXT: lui $2, 4
# CHECK-NEXT: lui $1, 0

# EMPTY-TABLE:      lui $4, 0
# EMPTY-TABLE-NEXT: lui $3, 0
# EMPTY-TABLE-NEXT: lui $2, 0
# EMPTY-TABLE-NEXT: lui $1, 0


nop


lui        $1, %hi(%neg(%captab_rel(__start)))
daddiu     $1, $1, %lo(%neg(%captab_rel(__start)))
cincoffset $c1, $c12, $1
# Address of __start is 0x20000 so the difference should be 0x20000:
# CHECK:   lui $1, 2
# CHECK:   daddiu  $1, $1, 0

# with an empty table this loads -addrof(__start) -> -0x20000
# EMPTY-TABLE: lui $1, 65534
# EMPTY-TABLE: daddiu  $1, $1, 0



# Just to ensure we have something in the captable:
.ifdef EMPTY_CAP_TABLE
nop
.else
clcbi      $c1, %captab20(bar)($c1)
.endif
cjr        $c17
cld        $2, $zero, 0($c1)


.ifdef FALSE
# This crashes the assembler:
daddiu     $1, $1, %lo(_CHERI_CAPABILITY_TABLE_ - __start)
daddiu     $1, $1, %hi(_CHERI_CAPABILITY_TABLE_ - __start)
daddiu     $1, $1, %higher(_CHERI_CAPABILITY_TABLE_ - __start)
daddiu     $1, $1, %highest(_CHERI_CAPABILITY_TABLE_ - __start)
.endif

.end __start


.ent fn_2
fn_2:
.set noat
lui        $1, %hi(%neg(%captab_rel(fn_2)))
daddiu     $1, $1, %lo(%neg(%captab_rel(fn_2)))
# address of fn2 = 0x2002c -> ﻿hex(0x40000 − 0x2002c) = 1ffd4
# -> lui 0x20000 and subtract 44
# CHECK: lui $1, 2
# CHECK: daddiu $1, $1, -44

# with an empty table this loads -addrof(fn_2) -> -0x2002c
# EMPTY-TABLE: lui $1, 65534
# EMPTY-TABLE: daddiu  $1, $1, -44

nop


lui        $1, %hi(%neg(%captab_rel(foo)))
daddiu     $1, $1, %lo(%neg(%captab_rel(foo)))
# address of foo = 0x30020 -> ﻿hex(0x40000 − 0x30020) = 0ffd0
# -> lui 0x10000 and subtract 32
# CHECK: lui $1, 1
# CHECK: daddiu $1, $1, -32

# with an empty table this loads -addrof(foo) -> -0x30020
# EMPTY-TABLE: lui $1, 65533
# EMPTY-TABLE: daddiu  $1, $1, -32

.end fn_2

.data
.space 32
foo:
.8byte 9
.size foo, 8
bar:
.8byte 10
.size bar, 8


# CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
# CHECK: 0x0000000000040000      Base: bar (0x0000000000030028)  Offset: 0x0000000000000000      Length: 0x0000000000000008     Permissions: 0x00000000


# CHECK-LABEL: Sections:
# CHECK: .text         00000040 0000000000020000 TEXT
# CHECK: .data         00000030 0000000000030000 DATA
# CHECK: .cap_table    00000010 0000000000040000 DATA

# CHECK-LABEL: SYMBOL TABLE:
# CHECK-NEXT:  0000000000000000         *UND*           00000000
# CHECK-NEXT:  0000000000030028         .data           00000008 bar
# CHECK-NEXT:  000000000002002c l     F .text           00000014 fn_2
# CHECK-NEXT:  0000000000030020         .data           00000008 foo
# CHECK-NEXT:  0000000000040000         .cap_table              00000000 _CHERI_CAPABILITY_TABLE_
# CHECK-NEXT:  0000000000038020         .got            00000000 .hidden _gp
# CHECK-NEXT:  0000000000040000 l       .cap_table              00000010 bar@CAPTABLE
# CHECK-NEXT:  0000000000020000 g     F .text           0000002c __start



