# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
#
# Check that the assembler is able to handle capability memory instructions.
#

# CHECK: csc	$cnull, $13, 48($c1)
# CHECK: encoding: [0xf8,0x01,0x68,0x03]
	csc		$cnull, $t1, 48($c1)
# CHECK: clc	$cnull, $13, 48($c1)
# CHECK: encoding: [0xd8,0x01,0x68,0x03]
	clc		$cnull, $t1, 48($c1)
# CHECK: clb	$12, $13, 3($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x1c]
	clb		$t0, $t1, 3($c1)
# CHECK: clh	$12, $13, 6($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x1d]
	clh		$t0, $t1, 6($c1)
# CHECK: clw	$12, $13, 12($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x1e]
	clw		$t0, $t1, 12($c1)
# CHECK: clbu	$12, $13, 3($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x18]
	clbu		$t0, $t1, 3($c1)
# CHECK: clhu	$12, $13, 6($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x19]
	clhu		$t0, $t1, 6($c1)
# CHECK: clwu	$12, $13, 12($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x1a]
	clwu		$t0, $t1, 12($c1)
# CHECK: cld	$12, $13, 24($c1)
# CHECK: encoding: [0xc9,0x81,0x68,0x1b]
	cld		$t0, $t1, 24($c1)
# CHECK: csb	$12, $13, 3($c1)
# CHECK: encoding: [0xe9,0x81,0x68,0x18]
	csb		$t0, $t1, 3($c1)
# CHECK: csh	$12, $13, 6($c1)
# CHECK: encoding: [0xe9,0x81,0x68,0x19]
	csh		$t0, $t1, 6($c1)
# CHECK: csw	$12, $13, 12($c1)
# CHECK: encoding: [0xe9,0x81,0x68,0x1a]
	csw		$t0, $t1, 12($c1)
# CHECK: csd	$12, $13, 24($c1)
# CHECK: encoding: [0xe9,0x81,0x68,0x1b]
	csd		$t0, $t1, 24($c1)
# CHECK: clld	$12, $c1
# CHECK: encoding: [0x4a,0x0c,0x08,0x0b]
	clld		$t0, $c1
# CHECK: cscd	$12, $23, $c1
# CHECK: encoding: [0x4a,0x17,0x0b,0x03]
	cscd		$t0, $s7, $c1
