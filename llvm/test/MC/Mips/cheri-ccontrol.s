# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
#
# Check that the assembler is able to handle capability branch instructions
# (and related things).
#

# CHECK: cbtu	$c1, 100
# CHECK: encoding: [0x49,0x21,0x00,0x19]
	cbtu	$c1, 100
# CHECK: cbts	$c1, 100
# CHECK: encoding: [0x49,0x41,0x00,0x19]
	cbts	$c1, 100
# CHECK: cbez	$c1, 100
# CHECK: encoding: [0x4a,0x21,0x00,0x19]
	cbez	$c1, 100
# CHECK: cbnz	$c1, 100
# CHECK: encoding: [0x4a,0x41,0x00,0x19]
	cbnz	$c1, 100
# CHECK: cjr	$c1
# CHECK: encoding: [0x48,0x01,0x1f,0xff]
	cjr	$c1
# CHECK: cjalr	$c1, $c4
# CHECK: encoding: [0x48,0x04,0x0b,0x3f]
	cjalr	$c1, $c4
# CHECK: ccall	$c2, $c3
# CHECK: encoding: [0x48,0xa2,0x18,0x00]
	ccall	$c2, $c3
# CHECK: ccall	$c2, $c3, 42
# CHECK: encoding: [0x48,0xa2,0x18,0x2a]
	ccall	$c2, $c3, 42
# CHECK: creturn
# CHECK: encoding: [0x48,0xa0,0x07,0xff]
	creturn
