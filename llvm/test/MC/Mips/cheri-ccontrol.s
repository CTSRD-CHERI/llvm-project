# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
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
# CHECK: cjr	$c1
# CHECK: encoding: [0x48,0x01,0x47,0xff]
	cjr	$c1
# CHECK: cjalr	$c4, $c1
# CHECK: encoding: [0x48,0x04,0x0b,0x3f]
	cjalr	$c4,$c1
# CHECK: ccall	$c2, $c3
# CHECK: encoding: [0x48,0xa2,0x18,0x00]
	ccall	$c2, $c3
# CHECK: creturn
# CHECK: encoding: [0x48,0xc0,0x00,0x00]
	creturn
