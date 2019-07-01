# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld %t.o -o %t.exe
# RUN: llvm-objdump -t -d -h -s -section=.data -section=.text %t.exe | FileCheck %s -check-prefix STATIC
# RUN: ld.lld -pie %t.o -o %t-pie.exe
# RUN: llvm-objdump -t -d -h -s -section=.data -section=.text %t-pie.exe | FileCheck %s -check-prefixes DYNAMIC,DYNAMIC-PIE
# RUN: ld.lld -shared %t.o -o %t.so
# RUN: llvm-objdump -t -d -h -s -section=.data -section=.text %t.so | FileCheck %s -check-prefixes DYNAMIC,DYNAMIC-SHLIB
# Also check that --export-dynamic doesn't cause _HAS__DYNAMIC to be true
# RUN: ld.lld -export-dynamic %t.o -o %t-export.exe
# RUN: llvm-objdump -t -d -h -s -section=.data -section=.text %t-export.exe | FileCheck %s -check-prefix STATIC

# Check that a static executable that is linked against a shared library has a valid _DYNAMIC symbol
# RUN: %cheri128_llvm-mc -filetype=obj %S/../Inputs/shared.s -o %t2.o
# RUN: ld.lld -shared %t2.o -o %t-shlib.so
# RUN: ld.lld %t.o %t-shlib.so -o %t-with-shlib.exe
# RUN: llvm-objdump -t -d -h -s -section=.data -section=.text %t-with-shlib.exe | FileCheck %s -check-prefixes DYNAMIC,DYNAMIC-NONPIC

# This is a hack since we can't easily load the value of _DYNAMIC prior to globals
# being initialized:
# Using %pcrel is 32-bits so it is not be enough on a 64-bit architecture and
# in PIC mode dla expands to a $gp relative value which also won't work for CHERI
.text
.option pic0
.global __start
.ent __start
__start:
ori $2, $0, %lo(_HAS__DYNAMIC)
.end __start

# STATIC-LABEL: Disassembly of section .text:
# STATIC-EMPTY:
# STATIC-NEXT: __start:
# STATIC-NEXT:   20000:	34 02 00 00 	ori	$2, $zero, 0

# DYNAMIC-LABEL: Disassembly of section .text:
# DYNAMIC-EMPTY:
# DYNAMIC-NEXT: __start:
# DYNAMIC-PIE-NEXT:      10000:	34 02 00 01 	ori	$2, $zero, 1
# DYNAMIC-SHLIB-NEXT:    10000:	34 02 00 01 	ori	$2, $zero, 1
# DYNAMIC-NONPIC-NEXT:   20000:	34 02 00 01 	ori	$2, $zero, 1

.data
.weak _DYNAMIC
.global value_of__DYNAMIC
value_of__DYNAMIC:
.8byte _DYNAMIC
.size value_of__DYNAMIC, 8

.8byte 0x1234567890abcdef
.global value_of_HAS__DYNAMIC
value_of_HAS__DYNAMIC:
.8byte _HAS__DYNAMIC
.size value_of_HAS__DYNAMIC, 8
.8byte 0xfedcba0987654321

# STATIC-LABEL: Contents of section .data:
# STATIC-NEXT: 30000 00000000 00000000 12345678 90abcdef
#                        ^----- _DYNAMIC == 0
# STATIC-NEXT: 30010 00000000 00000000 fedcba09 87654321
#                        ^----- _HAS__DYNAMIC == 0
# STATIC-LABEL: SYMBOL TABLE:

# STATIC-NEXT: 0000000000000000         *ABS*		 00000000 .hidden _HAS__DYNAMIC
# STATIC-NEXT: {{.+}}         .got		 00000000 .hidden _gp
# STATIC-NEXT: 0000000000000000  w      *UND*		 00000000 _DYNAMIC


# DYNAMIC-LABEL: Contents of section .data:
# DYNAMIC-PIE-NEXT:    20000 00000000 [[DYNAMIC_ADDR:00000[0-9a-f]+]] 12345678 90abcdef
# DYNAMIC-SHLIB-NEXT:  20000 00000000 [[DYNAMIC_ADDR:00000[0-9a-f]+]] 12345678 90abcdef
#                        ^----- _DYNAMIC == relocbase + 0x1f8 (addend 0x1f8 is written here since we use REL)
# DYNAMIC-NONPIC-NEXT: 30000 00000000 [[DYNAMIC_ADDR:000[0-9a-f]+]] 12345678 90abcdef
#                        ^----- absolute value for _DYNAMIC in non-pie executable with shlibs
# DYNAMIC-NEXT: {{3|2}}0010 00000000 00000001 fedcba09 87654321
#                        ^----- _HAS__DYNAMIC == 1
# DYNAMIC-LABEL: SYMBOL TABLE:
# DYNAMIC-PIE-NEXT:    00000000[[DYNAMIC_ADDR]]        .dynamic  00000000 .hidden _DYNAMIC
# DYNAMIC-SHLIB-NEXT:  00000000[[DYNAMIC_ADDR]]        .dynamic  00000000 .hidden _DYNAMIC
#                       ^----- _DYNAMIC == relocbase + 0x1f8/258
# DYNAMIC-NONPIC-NEXT: 00000000[[DYNAMIC_ADDR]]         .dynamic  00000000 .hidden _DYNAMIC
#                      ^----- absolute value for _DYNAMIC in non-pie executable with shlibs
# DYNAMIC-NEXT: 0000000000000001         *ABS*		 00000000 .hidden _HAS__DYNAMIC
#                       ^----- _HAS__DYNAMIC == 1
# DYNAMIC-NEXT: {{.+}}         .got		   00000000 .hidden _gp
