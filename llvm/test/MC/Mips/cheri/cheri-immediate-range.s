# RUN: not %cheri_llvm-mc %s -show-encoding 2> %t.err | FileCheck %s
# RUN: FileCheck %s -input-file=%t.err -check-prefix=ERR
# Check that the assembler uses the correct immediate ranges.
#


	cincoffset	$c1, $c2, 0
# CHECK: cincoffset	$c1, $c2, 0  # encoding: [0x4a,0x61,0x10,0x00]
	cincoffset	$c1, $c2, 1023
# CHECK: cincoffset	$c1, $c2, 1023  # encoding: [0x4a,0x61,0x13,0xff]
	cincoffset	$c1, $c2, -1024
# CHECK: cincoffset	$c1, $c2, -1024  # encoding: [0x4a,0x61,0x14,0x00]
	cincoffset	$c1, $c2, -1
# CHECK: cincoffset	$c1, $c2, -1  # encoding: [0x4a,0x61,0x17,0xff]
# CIncOffsetImmediate uses a signed 11 bit immediate:
# These are just out of range:
	cincoffset	$c1, $c2, 1024
# ERR: cheri-immediate-range.s:[[@LINE-1]]:23: error: expected 11-bit signed immediate
	cincoffset	$c1, $c2, -1025
# ERR: cheri-immediate-range.s:[[@LINE-1]]:23: error: expected 11-bit signed immediate


	csetbounds	$c1, $c2, 0
# CHECK: csetbounds	$c1, $c2, 0  # encoding: [0x4a,0x81,0x10,0x00]
	csetbounds	$c1, $c2, 1023
# CHECK: csetbounds	$c1, $c2, 1023  # encoding: [0x4a,0x81,0x13,0xff]
	csetbounds	$c1, $c2, 2047
# CHECK: csetbounds	$c1, $c2, 2047  # encoding: [0x4a,0x81,0x17,0xff]
# CSetBoundsImmediate uses an 11 unsigned immediate so these are just out of bounds
	csetbounds	$c1, $c2, 2048
# ERR: cheri-immediate-range.s:[[@LINE-1]]:23: error: expected 11-bit unsigned immediate
	csetbounds	$c1, $c2, -1
# ERR: cheri-immediate-range.s:[[@LINE-1]]:23: error: expected 11-bit unsigned immediate


ccall $c1, $c2, 2047
# CHECK: ccall	$c1, $c2, 2047          # encoding: [0x48,0xa1,0x17,0xff]
ccall $c1, $c2, 2048
# ERR: cheri-immediate-range.s:[[@LINE-1]]:17: error: expected 11-bit unsigned immediate
