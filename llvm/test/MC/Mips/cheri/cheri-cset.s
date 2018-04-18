# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: %cheri_llvm-mc %s -show-encoding 2>&1 -defsym=BAD=1| FileCheck %s -check-prefix=WARN
# Check that llvm-objdump doesn't crash the disassembler due to invalid register names in tablegen:
# RUN: %cheri_llvm-mc %s -filetype=obj -o - | llvm-objdump -d - > /dev/null
#
# Check that the assembler is able to handle capability get instructions.
#
# CHECK: cseal	$c1, $c2, $c3
# CHECK-SAME: encoding: [0x48,0x01,0x10,0xcb]
	cseal	$c1, $c2, $c3
# CHECK: cunseal	$c1, $c2, $c3
# CHECK-SAME: encoding: [0x48,0x01,0x10,0xcc]
	cunseal		$c1, $c2, $c3
# CHECK: candperm	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x0d]
	candperm	$c1, $c2, $t0
# CHECK: csetbounds	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x08]
	csetbounds	$c1, $c2, $t0
# CHECK: csetbounds	$c1, $c2, 64
# CHECK-SAME: encoding: [0x4a,0x81,0x10,0x40]
	csetbounds	$c1, $c2, 64
# CHECK: csetboundsexact	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x09]
	csetboundsexact	$c1, $c2, $t0
# CHECK: cincoffset	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x11]
	cincoffset	$c1, $c2, $t0
# CHECK: cincoffset	$c1, $c2, 64
# CHECK-SAME: encoding: [0x4a,0x61,0x10,0x40]
	cincoffset	$c1, $c2, 64
# CHECK: ccleartag	$c1
# CHECK-SAME: encoding: [0x48,0x01,0x0a,0xff]
	ccleartag	$c1, $c1
# CHECK: cmove	$c1, $c2
# CHECK-SAME: encoding: [0x48,0x01,0x12,0xbf]
	cmove	$c1, $c2
# CHECK: cmovz	$c1, $c3, $2
# CHECK-SAME: encoding: [0x48,0x01,0x18,0x9b]
	cmovz	$c1, $c3, $2
# CHECK: cmovn	$c1, $c3, $2
# CHECK-SAME: encoding: [0x48,0x01,0x18,0x9c]
	cmovn	$c1, $c3, $2

# Check for correct encoding of explicit set / get default and deprecated
# direct access to C0

# CHECK: csetdefault	$c1
# CHECK-SAME: encoding: [0x48,0x00,0x08,0x11]
	CSetDefault	$c1
# CHECK: cgetdefault	$c1
# CHECK-SAME: encoding: [0x48,0x01,0x00,0x11]
	CGetDefault	$c1

.ifdef BAD
# WARN: warning: Direct access to DDC is deprecated. Use C(Get/Set)Default instead.
# WARN-NEXT: CMove $c1, $c0
	CMove	$c1, $c0
# WARN: warning: Direct access to DDC is deprecated. Use C(Get/Set)Default instead.
# WARN-NEXT: CMove $c0, $c1
	CMove	$c0, $c1

# WARN: warning: Direct access to KR1C is deprecated. Use C(Get/Set)KR1C instead.
# WARN-NEXT:	CMove	$c1, $kr1c
	CMove	$c1, $kr1c
# WARN: warning: Direct access to KR1C is deprecated. Use C(Get/Set)KR1C instead.
# WARN-NEXT:	CMove	$kr1c, $c1
	CMove	$kr1c, $c1
# WARN: warning: Direct access to KR1C is deprecated. Use C(Get/Set)KR1C instead.
# WARN-NEXT:	CMove	$c27, $c1
	CMove	$c27, $c1
# WARN: warning: Direct access to KR1C is deprecated. Use C(Get/Set)KR1C instead.
# WARN-NEXT:	CMove	$c1, $c27
	CMove	$c1, $c27

# WARN: warning: Direct access to KR2C is deprecated. Use C(Get/Set)KR2C instead.
# WARN-NEXT:	CMove	$c1, $kr2c
	CMove	$c1, $kr2c
# WARN: warning: Direct access to KR2C is deprecated. Use C(Get/Set)KR2C instead.
# WARN-NEXT:	CMove	$kr2c, $c1
	CMove	$kr2c, $c1
# WARN: warning: Direct access to KR2C is deprecated. Use C(Get/Set)KR2C instead.
# WARN-NEXT:	CMove	$c28, $c1
	CMove	$c28, $c1
# WARN: warning: Direct access to KR2C is deprecated. Use C(Get/Set)KR2C instead.
# WARN-NEXT:	CMove	$c1, $c28
	CMove	$c1, $c28


# WARN: warning: Direct access to KCC is deprecated. Use C(Get/Set)KCC instead.
# WARN-NEXT:	CMove	$c1, $kcc
	CMove	$c1, $kcc
# WARN: warning: Direct access to KCC is deprecated. Use C(Get/Set)KCC instead.
# WARN-NEXT:	CMove	$kcc, $c1
	CMove	$kcc, $c1
# WARN: warning: Direct access to KCC is deprecated. Use C(Get/Set)KCC instead.
# WARN-NEXT:	CMove	$c29, $c1
	CMove	$c29, $c1
# WARN: warning: Direct access to KCC is deprecated. Use C(Get/Set)KCC instead.
# WARN-NEXT:	CMove	$c1, $c29
	CMove	$c1, $c29

# WARN: warning: Direct access to KDC is deprecated. Use C(Get/Set)KDC instead.
# WARN-NEXT:	CMove	$c1, $kdc
	CMove	$c1, $kdc
# WARN: warning: Direct access to KDC is deprecated. Use C(Get/Set)KDC instead.
# WARN-NEXT:	CMove	$kdc, $c1
	CMove	$kdc, $c1
# WARN: warning: Direct access to KDC is deprecated. Use C(Get/Set)KDC instead.
# WARN-NEXT:	CMove	$c30, $c1
	CMove	$c30, $c1
# WARN: warning: Direct access to KDC is deprecated. Use C(Get/Set)KDC instead.
# WARN-NEXT:	CMove	$c1, $c30
	CMove	$c1, $c30

# WARN: warning: Direct access to EPCC is deprecated. Use C(Get/Set)EPCC instead.
# WARN-NEXT:	CMove	$c1, $epcc
	CMove	$c1, $epcc
# WARN: warning: Direct access to EPCC is deprecated. Use C(Get/Set)EPCC instead.
# WARN-NEXT:	CMove	$epcc, $c1
	CMove	$epcc, $c1
# WARN: warning: Direct access to EPCC is deprecated. Use C(Get/Set)EPCC instead.
# WARN-NEXT:	CMove	$c31, $c1
	CMove	$c31, $c1
# WARN: warning: Direct access to EPCC is deprecated. Use C(Get/Set)EPCC instead.
# WARN-NEXT:	CMove	$c1, $c31
	CMove	$c1, $c31
.endif

# FIXME: should this be an error since it bypasses the warnings?
# but it uses createNumericReg(27, ...)
#	CMove	$c1, $27


# Check that we can assemble the new CBuildCap/CCopyType and CTestSubset instrs
	CBuildCap	$c1, $c2, $c8
# CHECK: cbuildcap	$c1, $c2, $c8
# CHECK-SAME: encoding: [0x48,0x01,0x12,0x1d]
	CCopyType	$c1, $c2, $c8
# CHECK: ccopytype	$c1, $c2, $c8
# CHECK-SAME: encoding: [0x48,0x01,0x12,0x1e]
	CCSeal	$c1, $c2, $c8
# CHECK: ccseal	$c1, $c2, $c8
# CHECK-SAME: encoding: [0x48,0x01,0x12,0x1f]
	CTestSubset	$at, $c2, $c8
# CHECK: ctestsubset	$1, $c2, $c8
# CHECK-SAME: encoding: [0x48,0x01,0x12,0x20]
