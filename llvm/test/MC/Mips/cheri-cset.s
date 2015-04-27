# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri 2>&1 | FileCheck %s -check-prefix=WARN
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: cseal	$c1, $c2, $c3
# CHECK: encoding: [0x48,0x41,0x10,0xc0]
	cseal	$c1, $c2, $c3
# CHECK: cunseal	$c1, $c2, $c3
# CHECK: encoding: [0x48,0x61,0x10,0xc0]
	cunseal		$c1, $c2, $c3
# CHECK: candperm	$c1, $c2, $12
# CHECK: encoding: [0x48,0x81,0x13,0x00]
	candperm	$c1, $c2, $t0
# CHECK: csetbounds	$c1, $c2, $12
# CHECK: encoding: [0x48,0x21,0x13,0x00]
	csetbounds	$c1, $c2, $t0
# CHECK: cincbase	$c1, $c2, $12
# CHECK: encoding: [0x48,0x81,0x13,0x02]
	cincbase	$c1, $c2, $t0
# CHECK: csetlen	$c1, $c2, $12
# CHECK: encoding: [0x48,0x81,0x13,0x03]
	csetlen		$c1, $c2, $t0
# CHECK: ccleartag	$c1
# CHECK: encoding: [0x48,0x81,0x08,0x05]
	ccleartag	$c1, $c1

# Check for correct encoding of explicit set / get default and deprecated
# direct access to C0

# CHECK: csetdefault	$c1
# CHECK: encoding: [0x48,0x80,0x08,0x02]
	CSetDefault	$c1
# CHECK: cgetdefault	$c1
# CHECK: encoding: [0x48,0x81,0x00,0x02]
	CGetDefault	$c1
# WARN: warning: Direct access to c0 is deprecated.
# WARN-NEXT: CIncBase $c0, $c1, $0
# WARN-NEXT:           ^
# CHECK: csetdefault	 $c1
# CHECK: encoding: [0x48,0x80,0x08,0x02]
	CIncBase	$c0, $c1, $0

