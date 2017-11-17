# REQUIRES: mips
# RUN:	llvm-mc -filetype=obj -triple=mips-unknown-freebsd %s -o %t
# RUN:	llvm-mc -filetype=obj -triple=mips-unknown-freebsd %S/Inputs/thunk-gen-mips-target.s -o %t1

# SECTIONS command with the first pattern that does not match.
# Linking a PIC and non-PIC object files triggers the LA25 thunk generation.
# RUN:		echo "SECTIONS { \
# RUN:		.text : { \
# RUN:			*(.nomatch) \
# RUN:			%t(.text) \
# RUN:			. = . + 0x100000 ; \
# RUN:			%t1(.text) \
# RUN:		} \
# RUN:	}" > %t.script
# RUN: ld.lld -o %t2 --script %t.script %t %t1
# RUN: llvm-objdump -t %t2 | FileCheck %s
# CHECK: SYMBOL TABLE:
# CHECK-ANY: 00000000         .text           00000000 _start
# CHECK-ANY: 0010000c l     F .text           00000010 __LA25Thunk_too_far
# CHECK-ANY: 00100020 g     F .text           00000024 too_far

.global _start
_start:
	j too_far
	nop
