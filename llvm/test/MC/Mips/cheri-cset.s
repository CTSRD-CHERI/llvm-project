# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: csealdata	$c1, $c2, $c3
	csealdata	$c1, $c2, $c3
# CHECK: cunseal	$c1, $c2, $c3
	cunseal		$c1, $c2, $c3
# CHECK: csealcode	$c1, $c2
	csealcode	$c1, $c2
# CHECK: candperm	$c1, $c2, $t0
	candperm	$c1, $c2, $t0
# CHECK: csettype	$c1, $c2, $t0
	csettype	$c1, $c2, $t0
# CHECK: cincbase	$c1, $c2, $t0
	cincbase	$c1, $c2, $t0
# CHECK: csetlen	$c1, $c2, $t0
	csetlen		$c1, $c2, $t0
# CHECK: ccleartag	$c1
	ccleartag	$c1
