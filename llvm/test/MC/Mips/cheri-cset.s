# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri 2>&1 | FileCheck %s -check-prefix=WARN
#
# Check that the assembler is able to handle capability get instructions.
#

# CHECK: cseal	$c1, $c2, $c3
# CHECK: encoding: [0x48,0x01,0x10,0xcb]
	cseal	$c1, $c2, $c3
# CHECK: cunseal	$c1, $c2, $c3
# CHECK: encoding: [0x48,0x01,0x10,0xcc]
	cunseal		$c1, $c2, $c3
# CHECK: candperm	$c1, $c2, $12
# CHECK: encoding: [0x48,0x01,0x13,0x0d]
	candperm	$c1, $c2, $t0
# CHECK: csetbounds	$c1, $c2, $12
# CHECK: encoding: [0x48,0x01,0x13,0x10]
	csetbounds	$c1, $c2, $t0
# CHECK: ccleartag	$c1
# CHECK: encoding: [0x48,0x01,0x0a,0xff]
	ccleartag	$c1, $c1
# CHECK: cmove	$c1, $c2
# CHECK: encoding: [0x48,0x01,0x12,0xbf]
cmove	$c1, $c2
# CHECK: ccmovz	$c1, $c3, $2
# CHECK: encoding: [0x48,0x01,0x18,0x9b]
ccmovz	$c1, $c3, $2
# CHECK: ccmovn	$c1, $c3, $2
# CHECK: encoding: [0x48,0x01,0x18,0x9c]
ccmovn	$c1, $c3, $2

# Check for correct encoding of explicit set / get default and deprecated
# direct access to C0

# CHECK: csetdefault	$c1
# CHECK: encoding: [0x48,0x00,0x08,0x11]
	CSetDefault	$c1
# CHECK: cgetdefault	$c1
# CHECK: encoding: [0x48,0x01,0x00,0x11]
	CGetDefault	$c1
# WARN: warning: Direct access to c0 is deprecated.
# WARN-NEXT: CIncOffset $c0, $c1, $0
# WARN-NEXT:           ^
# CHECK: csetdefault	 $c1
# CHECK: encoding: [0x48,0x00,0x08,0x11]
	CIncOffset	$c0, $c1, $0

