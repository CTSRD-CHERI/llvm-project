# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability branch instructions
# (and related things).
#

# CHECK: cbtu	$c1, 100
	cbtu	$c1, 100
# CHECK: cbts	$c1, 100
	cbts	$c1, 100
# CHECK: cjr	$12($c1)
	cjr	$t0($c1)
# CHECK: cjalr	$12($c1)
	cjalr	$t0($c1)
# CHECK: ccall	$c2, $c3
	ccall	$c2, $c3
# CHECK: creturn
	creturn
