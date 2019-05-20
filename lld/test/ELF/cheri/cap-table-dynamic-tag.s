# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -z lazy -pie %t.o -o %t.exe
# RUN: llvm-readelf --program-headers --section-mapping %t.exe | FileCheck %s -check-prefix LAZY-SEGMENTS
# RUN: llvm-readobj --cap-relocs -dynamic-table %t.exe | FileCheck %s -check-prefixes CHECK,WITH-TABLE,WITH-TABLE-LAZY
# LAZY-SEGMENTS-LABEL: Section to Segment mapping:
# LAZY-SEGMENTS-NEXT:  Segment Sections...
# LAZY-SEGMENTS-NEXT:   00
# __cap_relocs should be in the rodata segment (since it uses relative relocations)
# LAZY-SEGMENTS-NEXT:   01 .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr __cap_relocs {{$}}
# LAZY-SEGMENTS-NEXT:   02 .text {{$}}
# .captable should be in the R/W data segment for lazy binding
# LAZY-SEGMENTS-NEXT:   03 .data .captable .rld_map .got {{$}}

# RUN: ld.lld -z now -pie %t.o -o %t.exe
# RUN: llvm-readobj --cap-relocs -dynamic-table %t.exe | FileCheck %s -check-prefixes CHECK,WITH-TABLE,WITH-TABLE-NOW
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=EMPTY_CAP_TABLE=1 %s -o %t2.o
# RUN: ld.lld -pie %t2.o -o %t.exe
# RUN: llvm-readobj --cap-relocs -dynamic-table %t.exe | FileCheck %s -check-prefixes CHECK,EMPTY-TABLE

.text


.global __start
.ent __start
__start:
.set noat
.set noreorder
lui        $1, %hi(%neg(%captab_rel(_CHERI_CAPABILITY_TABLE_)))
daddiu     $1, $1, %lo(%neg(%captab_rel(_CHERI_CAPABILITY_TABLE_)))
nop

.ifdef EMPTY_CAP_TABLE
nop
.else
# Just to ensure we have something in the captable:
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


# CHECK-LABEL: DynamicSection [
# WITH-TABLE-LAZY-SAME: (19 entries)
# WITH-TABLE-NOW-SAME: (21 entries)
# EMPTY-TABLE-SAME: (15 entries)
# CHECK-NEXT:    Tag                Type                 Name/Value
# WITH-TABLE-NOW-NEXT: 0x000000000000001E FLAGS BIND_NOW
# WITH-TABLE-NOW-NEXT: 0x000000006FFFFFFB FLAGS_1 NOW
# CHECK-NEXT:    0x0000000000000015 DEBUG                0x0
# CHECK-NEXT:    0x0000000000000006 SYMTAB               {{0x2B0|0x240}}
# CHECK-NEXT:    0x000000000000000B SYMENT               24 (bytes)
# CHECK-NEXT:    0x0000000000000005 STRTAB
# CHECK-NEXT:    0x000000000000000A STRSZ                1 (bytes)
# CHECK-NEXT:    0x0000000000000004 HASH
# CHECK-NEXT:    0x0000000070000001 MIPS_RLD_VERSION     1
# CHECK-NEXT:    0x0000000070000005 MIPS_FLAGS           NOTPOT
# CHECK-NEXT:    0x0000000070000006 MIPS_BASE_ADDRESS    0x0
# CHECK-NEXT:    0x0000000070000011 MIPS_SYMTABNO        1
# CHECK-NEXT:    0x000000007000000A MIPS_LOCAL_GOTNO     2
# CHECK-NEXT:    0x0000000070000013 MIPS_GOTSYM          0x1
# CHECK-NEXT:    0x0000000000000003 PLTGOT               0x{{2|3}}00{{4|5}}0
# CHECK-NEXT:    0x0000000070000035 MIPS_RLD_MAP_REL
# WITH-TABLE-NOW-NEXT: 0x000000007000C003 MIPS_CHERI_CAPTABLE  0x20000
# With lazy binding the captable needs to be in the writable area
# WITH-TABLE-LAZY-NEXT: 0x000000007000C003 MIPS_CHERI_CAPTABLE  0x20030
# WITH-TABLE-NEXT: 0x000000007000C004 MIPS_CHERI_CAPTABLESZ0x10
# WITH-TABLE-NOW-NEXT: 0x000000007000C000 MIPS_CHERI___CAPRELOCS0x430
# WITH-TABLE-LAZY-NEXT: 0x000000007000C000 MIPS_CHERI___CAPRELOCS0x3A0
# WITH-TABLE-NEXT: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ0x28
# CHECK-NEXT:    0x0000000000000000 NULL                 0x0
# CHECK-NEXT:  ]



