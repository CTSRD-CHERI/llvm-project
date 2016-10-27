# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: cgetperm	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x00]
	cgetperm	$t0, $c1
# CHECK: cgettype	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x01]
	cgettype	$t0, $c1
# CHECK: cgetbase	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x02]
	cgetbase	$t0, $c1
# CHECK: cgetlen	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x03]
	cgetlen		$t0, $c1
# CHECK: cgettag	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x05]
	cgettag		$t0, $c1
# CHECK: cgetsealed	$12, $c1
# CHECK: encoding: [0x48,0x0c,0x08,0x06]
	cgetsealed	$t0, $c1
# CHECK: cgetpcc	$c1
# CHECK: encoding: [0x48,0x01,0x07,0xff]
	cgetpcc		$c1
# CHECK: cgetcause	$12
# CHECK: encoding: [0x48,0x0c,0x00,0x04]
	cgetcause	$t0
# CHECK: cgetepcc	 $
# CHECK:  encoding: [0x49,0xa4,0xf8,0x00]
	CGetEPCC $c4
# CHECK: csetepcc	 $
# CHECK:  encoding: [0x49,0xbf,0x20,0x00]
	CSetEPCC $c4
# CHECK: cgetkcc	 $
# CHECK:  encoding: [0x49,0xa4,0xe8,0x00]
	CGetKCC $c4
# CHECK: csetkcc	 $
# CHECK:  encoding: [0x49,0xbd,0x20,0x00]
	CSetKCC $c4

# CHECK: csub	 $1, $c2, $c3
# CHECK:  encoding: [0x48,0x01,0x10,0xca]
	csub	$1, $c2, $c3

