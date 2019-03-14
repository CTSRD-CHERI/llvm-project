# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: not %cheri_llvm-mc %s -show-encoding 2>&1 -defsym=BAD=1 | FileCheck %s -check-prefix=WARN
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
# CHECK: csetbounds	$c1, $c2, 65
# CHECK-SAME: encoding: [0x4a,0x81,0x10,0x41]
	csetboundsimm	$c1, $c2, 65
# CHECK: csetboundsexact	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x09]
	csetboundsexact	$c1, $c2, $t0
# CHECK: cincoffset	$c1, $c2, $12
# CHECK-SAME: encoding: [0x48,0x01,0x13,0x11]
	cincoffset	$c1, $c2, $t0
# CHECK: cincoffset	$c1, $c2, 64
# CHECK-SAME: encoding: [0x4a,0x61,0x10,0x40]
	cincoffset	$c1, $c2, 64
# CHECK: cincoffset	$c1, $c2, 65
# CHECK-SAME: encoding: [0x4a,0x61,0x10,0x41]
	cincoffsetimm	$c1, $c2, 65
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

# CHECK: cwritehwr $c1, $chwr_ddc
# CHECK-SAME: encoding: [0x48,0x01,0x03,0xbf]
	CSetDefault	$c1
# CHECK: creadhwr $c1, $chwr_ddc
# CHECK-SAME: encoding: [0x48,0x01,0x03,0x7f]
	CGetDefault	$c1

.ifdef BAD
# WARN: error: Register $kr1c is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kr1c instead. If you really want to access $c27 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
	CMove	$c1, $kr1c
# WARN: error: Register $kr1c is no longer a general-purpose CHERI register. If you want to access the special register use c{get,set}kr1c instead. If you really want to access $c27 (which is ABI-reserved for kernel use and may be clobbered at any time), use that instead.
	CMove	$kr1c, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c27, $c1
	CMove	$c27, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c1, $c27
	CMove	$c1, $c27

# WARN: error: Register $kr2c is no longer a general-purpose CHERI register.
	CMove	$c1, $kr2c
# WARN: error: Register $kr2c is no longer a general-purpose CHERI register.
	CMove	$kr2c, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c28, $c1
	CMove	$c28, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c1, $c28
	CMove	$c1, $c28

# WARN: error: Register $kcc is no longer a general-purpose CHERI register.
	CMove	$c1, $kcc
# WARN: error: Register $kcc is no longer a general-purpose CHERI register.
	CMove	$kcc, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c29, $c1
	CMove	$c29, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c1, $c29
	CMove	$c1, $c29

# WARN: error: Register $kdc is no longer a general-purpose CHERI register.
	CMove	$c1, $kdc
# WARN: error: Register $kdc is no longer a general-purpose CHERI register.
	CMove	$kdc, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c30, $c1
	CMove	$c30, $c1
# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c1, $c30
	CMove	$c1, $c30

# EPCC can no longer be accessed using $c31
# WARN: error: Register $epcc is no longer a general-purpose CHERI register.
	CMove	$c1, $epcc
# WARN: error: Register $epcc is no longer a general-purpose CHERI register.
	CMove	$epcc, $c1
# WARN: [[@LINE+2]]:8: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c31, $c1
	CMove	$c31, $c1
# WARN: [[@LINE+2]]:13: warning: Registers $c27 to $c31 are ABI-reserved for kernel use
# WARN-NEXT:	CMove	$c1, $c31
	CMove	$c1, $c31

# Also don't allow just numeric registers since that bypasses the checks added in
# matchCheriRegisterName
# WARN: [[@LINE+2]]:8: error: expected general-purpose CHERI register operand
# WARN-NEXT:	CMove	$31, $c1
	CMove	$31, $c1
# WARN: [[@LINE+2]]:13: error: expected general-purpose CHERI register operand
# WARN-NEXT:	CMove	$c1, $31
	CMove	$c1, $31

# DDC should not suggest cheri_sysregs_accessible
# WARN: [[@LINE+3]]:13: error: register name $c0 is invalid as this operand. It will no longer refer to $ddc but instead be NULL.
# WARN-NOT: cheri_sysregs_accessible
# WARN-NEXT: CMove $c1, $c0
	CMove	$c1, $c0
# WARN: [[@LINE+3]]:8: error: register name $c0 is invalid as this operand. It will no longer refer to $ddc but instead be NULL.
# WARN-NOT: cheri_sysregs_accessible
# WARN-NEXT: CMove $c0, $c1
	CMove	$c0, $c1

	# Check that there are no warnings for KR1C/KR2C/KDC/KCC with cheri_sysregs_accessible
	.set cheri_sysregs_accessible
	# WARN-NOT: warning:
	# WARN-NOT: error:
	CMove	$c1, $c27
	CMove	$c1, $c28
	CMove	$c1, $c29
	CMove	$c1, $c30
	CMove	$c1, $c31

  # And $C0 should also not have changed:
  # WARN: [[@LINE+3]]:13: error: register name $c0 is invalid as this operand. It will no longer refer to $ddc but instead be NULL.
  # WARN-NOT: cheri_sysregs_accessible
  # WARN-NEXT: CMove $c1, $c0
	CMove	$c1, $c0

  # Check that disabling works:
	.set nocheri_sysregs_accessible
	# WARN: warning: Registers $c27 to $c31 are ABI-reserved for kernel use and may be clobbered at any time.
  # WARN-SAME: If you are writing kernel code and really meant to access $c27 you can use `.set cheri_sysregs_accessible` to silence this warning.
  # WARN-NEXT:	CMove	$c1, $c27
	CMove	$c1, $c27


	# Typo in the .set:
	# WARN: [[@LINE+1]]:34: error: unexpected token, expected comma
	.set no_cheri_sysregs_accessible
.endif

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
