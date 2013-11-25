# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: cgetperm	$t0, $c1
	cgetperm	$t0, $c1
# CHECK: cgettype	$t0, $c1
	cgettype	$t0, $c1
# CHECK: cgetbase	$t0, $c1
	cgetbase	$t0, $c1
# CHECK: cgetlen	$t0, $c1
	cgetlen		$t0, $c1
# CHECK: cgettag	$t0, $c1
	cgettag		$t0, $c1
# CHECK: cgetunsealed	$t0, $c1
	cgetunsealed	$t0, $c1
# CHECK: cgetpcc	$t0($c1)
	cgetpcc		$t0($c1)
