# RUN: %cheri128_purecap_llvm-mc %s -defsym=FILE=1 -filetype=obj -o %t1.o
# RUN: %cheri128_purecap_llvm-mc %s -defsym=FILE=2 -filetype=obj -o %t2.o
# RUN: %cheri128_purecap_llvm-mc %s -defsym=FILE=3 -filetype=obj -o %t-duplicate.o

# Should not be possible to have duplicate names for globals in .captable
# RUN: not ld.lld %t1.o %t2.o %t-duplicate.o -o %t.exe 2>&1 | FileCheck %s -check-prefix ERR
# ERR: error: duplicate symbol: duplicate_global
# ERR-NEXT: >>> defined at {{.+}}caprelocs-duplicate-symbol-names.s.tmp1.o:(duplicate_global)
# ERR-NEXT: >>> defined at {{.+}}caprelocs-duplicate-symbol-names.s.tmp-duplicate.o:(.data+0x0)

# But duplicate locals are fine so we need to add sensible @CAPTABLE names:
# RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t1.o %t2.o -o %t.exe
# RUN: llvm-objdump --cap-relocs -d -r -t %t.exe | FileCheck %s

# Check that we still have all the capability relocations:
# See https://github.com/CTSRD-CHERI/lld/issues/19

# CHECK-LABEL: SYMBOL TABLE:
# CHECK:      0000000000030410 l     O .captable		 0000000000000010 .L.str.123@CAPTABLE.0
# CHECK-NEXT: 0000000000030420 l     O .captable		 0000000000000010 .L.duplicate_local@CAPTABLE.1
# CHECK-NEXT: 0000000000030430 l     O .captable		 0000000000000010 duplicate_local_without_prefix@CAPTABLE.2
# CHECK-NEXT: 0000000000030440 l     O .captable		 0000000000000010 duplicate_global@CAPTABLE
# CHECK-NEXT: 0000000000030450 l     O .captable		 0000000000000010 local_with_same_name_as_global@CAPTABLE.4
# CHECK-NEXT: 0000000000030460 l     O .captable		 0000000000000010 .L.duplicate_local@CAPTABLE.5
# CHECK-NEXT: 0000000000030470 l     O .captable		 0000000000000010 duplicate_local_without_prefix@CAPTABLE.6
# CHECK-NEXT: 0000000000030480 l     O .captable		 0000000000000010 g@CAPTABLE
# CHECK-NEXT: 0000000000030490 l     O .captable		 0000000000000010 local_with_same_name_as_global@CAPTABLE
# CHECK:      0000000000030410 l       .captable		 0000000000000090 _CHERI_CAPABILITY_TABLE_

# CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
# CHECK-NEXT: 0x0000000000030410	Base: .L.str.123 (0x00000000000404a0)	Offset: 0x0000000000000000	Length: 0x0000000000000006	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030420	Base: .L.duplicate_local (0x00000000000404a6)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030430	Base: duplicate_local_without_prefix (0x00000000000404ae)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030440	Base: duplicate_global (0x00000000000404b6)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030450	Base: local_with_same_name_as_global (0x00000000000404be)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030460	Base: .L.duplicate_local (0x00000000000404d0)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030470	Base: duplicate_local_without_prefix (0x00000000000404d8)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030480	Base: g (0x00000000000404e0)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030490	Base: local_with_same_name_as_global (0x00000000000404e8)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000

.macro add_captable_reference name
.text
clcbi $c1, %captab20(\name)($c1)  # add a reference so that it ends up in the captable
.data
.endm

.macro add_data_symbol name, value:vararg
.data
\name:
\value
.L\name\().end:
.size \name, .L\name\().end - \name

.endm


.if FILE == 1

.text
.global __start
__start:
  nop

add_data_symbol .L.str.123, .asciiz "asdfg"
add_captable_reference .L.str.123

# Try a duplicate local name:
add_data_symbol .L.duplicate_local, .quad 8
add_captable_reference .L.duplicate_local

add_data_symbol duplicate_local_without_prefix, .quad 40
add_captable_reference duplicate_local_without_prefix

.global duplicate_global
add_data_symbol duplicate_global, .quad 10
add_captable_reference duplicate_global


# define a local symbol with the same name as a global one defined in file 2
# Check that this doesn't break the @CAPTABLE naming
add_data_symbol local_with_same_name_as_global, .quad 10
add_captable_reference local_with_same_name_as_global

.endif

.if FILE == 2
add_data_symbol .L.duplicate_local, .quad 9
add_captable_reference .L.duplicate_local

add_data_symbol duplicate_local_without_prefix, .quad 42
add_captable_reference duplicate_local_without_prefix

.global g
add_data_symbol g, .quad 10
add_captable_reference g

.global local_with_same_name_as_global
add_data_symbol local_with_same_name_as_global, .quad 10
add_captable_reference local_with_same_name_as_global


.endif

.if FILE == 3
.global duplicate_global
add_data_symbol duplicate_global, .quad 10
add_captable_reference duplicate_global
.endif
