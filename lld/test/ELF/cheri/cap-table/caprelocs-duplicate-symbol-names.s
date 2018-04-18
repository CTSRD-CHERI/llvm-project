# RUN: %cheri_purecap_llvm-mc %s -defsym=FILE=1 -filetype=obj -o %t1.o
# RUN: %cheri_purecap_llvm-mc %s -defsym=FILE=2 -filetype=obj -o %t2.o
# RUN: %cheri_purecap_llvm-mc %s -defsym=FILE=3 -filetype=obj -o %t-duplicate.o
# Should not be possible to have duplicate names for globals in .captable
# RUN: not ld.lld -shared %t1.o %t2.o %t-duplicate.o -o %t.so 2>&1 | FileCheck %s -check-prefix ERR
# ERR: error: duplicate symbol: duplicate_global
# ERR-NEXT: >>> defined at {{.+}}caprelocs-duplicate-symbol-names.s.tmp1.o:(duplicate_global)
# ERR-NEXT: >>> defined at {{.+}}caprelocs-duplicate-symbol-names.s.tmp-duplicate.o:(.data+0x0)

# But duplicate locals are fine so we need to add sensible @CAPTABLE names:
# RUN: ld.lld -shared -preemptible-caprelocs=legacy %t1.o %t2.o -o %t.so
# RUN: llvm-objdump -C -d -r -t %t.so
# RUN: llvm-objdump -C -d -r -t %t.so | FileCheck %s

# Check that we still have all the capability relocations:
# See https://github.com/CTSRD-CHERI/lld/issues/19

# CHECK: CAPABILITY RELOCATION RECORDS:
# CHECK-NEXT: 0x0000000000030000	Base: .L.str.123 (0x0000000000020000)	Offset: 0x0000000000000000	Length: 0x0000000000000006	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030010	Base: .L.duplicate_local (0x0000000000020006)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030020	Base: duplicate_local_without_prefix (0x000000000002000e)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030030	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030040	Base: .L.duplicate_local (0x0000000000020020)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030050	Base: duplicate_local_without_prefix (0x0000000000020028)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# CHECK-NEXT: 0x0000000000030060	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000

# CHECK: SYMBOL TABLE:
# CHECK:      0000000000030000         .cap_table		 00000070 _CHERI_CAPABILITY_TABLE_
# CHECK-NEXT: 0000000000030000 l       .cap_table		 00000010 .L.str.123@CAPTABLE.0
# CHECK-NEXT: 0000000000030010 l       .cap_table		 00000010 .L.duplicate_local@CAPTABLE.1
# CHECK-NEXT: 0000000000030020 l       .cap_table		 00000010 duplicate_local_without_prefix@CAPTABLE
# CHECK-NEXT: 0000000000030030 l       .cap_table		 00000010 duplicate_global@CAPTABLE
# CHECK-NEXT: 0000000000030040 l       .cap_table		 00000010 .L.duplicate_local@CAPTABLE.4
# CHECK-NEXT: 0000000000030050 l       .cap_table		 00000010 duplicate_local_without_prefix@CAPTABLE.5

.macro add_captable_reference name
.text
clcbi $c1, %captab20(\name)($c1)  # add a reference so that it ends up in the captable
.data
.endm

.macro add_data_symbol name, value:vararg
.data
\name:
\value
\name\().end:
.size \name, \name\().end - \name

.endm


.if FILE == 1
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
.endif

.if FILE == 2
add_data_symbol .L.duplicate_local, .quad 9
add_captable_reference .L.duplicate_local

add_data_symbol duplicate_local_without_prefix, .quad 42
add_captable_reference duplicate_local_without_prefix

.global g
add_data_symbol g, .quad 10
add_captable_reference g
.endif

.if FILE == 3
.global duplicate_global
add_data_symbol duplicate_global, .quad 10
add_captable_reference duplicate_global
.endif
