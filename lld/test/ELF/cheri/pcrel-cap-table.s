# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -z now %t.o -o %t.exe
# RUN: llvm-objdump --no-print-imm-hex --cap-relocs -t -d -h %t.exe | FileCheck %s
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=EMPTY_CAP_TABLE=1 %s -o %t2.o
# RUN: ld.lld -z now %t2.o -o %t.exe
# RUN: llvm-objdump --no-print-imm-hex --cap-relocs -t -d -h %t.exe | FileCheck %s --check-prefix EMPTY-TABLE

.text

# CHECK-LABEL: Sections:
# CHECK: .text         00000040 00000000000202a0 TEXT
# CHECK: .captable     00000010 00000000000302e0 DATA
# CHECK: .data         00000030 00000000000402f0 DATA

# CHECK-LABEL: SYMBOL TABLE:
# CHECK-NEXT:  0000000000040318 l       .data           0000000000000008 bar
# CHECK-NEXT:  00000000000202cc l     F .text           0000000000000014 fn_2
# CHECK-NEXT:  0000000000040310 l       .data           0000000000000008 foo
# CHECK-NEXT:  00000000000302e0 l     O .captable       0000000000000010 bar@CAPTABLE
# CHECK-NEXT:  00000000000302e0 l       .captable       0000000000000000 .hidden _CHERI_CAPABILITY_TABLE_
# CHECK-NEXT:  0000000000048310 l       .got            0000000000000000 .hidden _gp
# CHECK-NEXT:  00000000000202a0 g     F .text           000000000000002c __start

# CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
# CHECK: 0x00000000000302e0      Base: bar (0x0000000000040318)  Offset: 0x0000000000000000      Length: 0x0000000000000008     Permissions: 0x00000000

# CHECK-LABEL: Disassembly of section .text:

.global __start
.ent __start
__start:
# CHECK-LABEL: <__start>:
.set noat
.set noreorder
# TODO: check for overflow?

lui        $4, %highest(_CHERI_CAPABILITY_TABLE_)
lui        $3, %higher(_CHERI_CAPABILITY_TABLE_)
lui        $2, %hi(_CHERI_CAPABILITY_TABLE_)
lui        $1, %lo(_CHERI_CAPABILITY_TABLE_)
# Address is 0x302e0:
# CHECK:      lui $4, 0
# CHECK-NEXT: lui $3, 0
# CHECK-NEXT: lui $2, 3
# CHECK-NEXT: lui $1, 736

# EMPTY-TABLE:      lui $4, 0
# EMPTY-TABLE-NEXT: lui $3, 0
# EMPTY-TABLE-NEXT: lui $2, 0
# EMPTY-TABLE-NEXT: lui $1, 0


nop


lui        $1, %hi(%neg(%captab_rel(__start)))
daddiu     $1, $1, %lo(%neg(%captab_rel(__start)))
cincoffset $c1, $c12, $1
# Address of __start is 0x202a0 so the difference should be 0x10040:
# CHECK:   lui $1, 1
# CHECK:   daddiu  $1, $1, 64

# with an empty table this loads -addrof(__start) -> -0x20210
# EMPTY-TABLE: lui $1, 65534
# EMPTY-TABLE: daddiu  $1, $1, -528



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
# address of fn2 = 0x202cc -> hex(0x302e0 − 0x202cc) = 0x10014
# CHECK: lui $1, 1
# CHECK: daddiu $1, $1, 20

# with an empty table this loads -addrof(fn_2) -> -0x2023c
# EMPTY-TABLE: lui $1, 65534
# EMPTY-TABLE: daddiu  $1, $1, -572

nop


lui        $1, %hi(%neg(%captab_rel(foo)))
daddiu     $1, $1, %lo(%neg(%captab_rel(foo)))
# address of foo = 0x40310 -> hex(0x302e0 − 0x40310) = -0x10030
# -> lui -1 and subtract 48
# CHECK: lui $1, 65535
# CHECK: daddiu $1, $1, -48

# with an empty table this loads -addrof(foo) -> -0x30270
# EMPTY-TABLE: lui $1, 65533
# EMPTY-TABLE: daddiu  $1, $1, -624

.end fn_2

.data
.space 32
foo:
.8byte 9
.size foo, 8
bar:
.8byte 10
.size bar, 8
