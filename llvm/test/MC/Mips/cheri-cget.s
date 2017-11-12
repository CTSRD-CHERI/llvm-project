# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: cgetperm	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x3f]
	cgetperm	$t0, $c1
# CHECK: cgettype	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x7f]
	cgettype	$t0, $c1
# CHECK: cgetbase	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0xbf]
	cgetbase	$t0, $c1
# CHECK: cgetlen	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0xff]
	cgetlen		$t0, $c1
# CHECK: cgettag	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x09,0x3f]
	cgettag		$t0, $c1
# CHECK: cgetsealed	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x09,0x7f]
	cgetsealed	$t0, $c1
# CHECK: cgetpcc	$c1
# CHECK: encoding: [0x48,0x01,0x07,0xff]
	cgetpcc		$c1
# CHECK: cgetpccsetoffset	$c1, $3
# CHECK: encoding: [0x48,0x01,0x19,0xff]
	cgetpccsetoffset		$c1, $3
# Check that this doesn't give a CGetPCC
# CHECK: cgetpccsetoffset	$c1, $zero
# CHECK: encoding: [0x48,0x01,0x01,0xff]
	cgetpccsetoffset		$c1, $zero
# CHECK: cgetcause	$12
# CHECK: encoding: [0x48,0x0c,0x0f,0xff]
	cgetcause	$t0
# CHECK: cgetepcc	 $c4
# CHECK:  encoding: [0x48,0x04,0xf8,0x11]
	CGetEPCC $c4
# CHECK: csetepcc	 $c4
# CHECK:  encoding: [0x48,0x1f,0x20,0x11]
	CSetEPCC $c4
# CHECK: cgetkcc	 $c4
# CHECK:  encoding: [0x48,0x04,0xe8,0x11]
	CGetKCC $c4
# CHECK: csetkcc	 $c4
# CHECK:  encoding: [0x48,0x1d,0x20,0x11]
	CSetKCC $c4

# CHECK: csub	 $1, $c2, $c3
# CHECK:  encoding: [0x48,0x01,0x10,0xca]
	csub	$1, $c2, $c3

