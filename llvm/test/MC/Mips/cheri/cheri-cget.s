# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: %cheri_llvm-mc %s -filetype=obj -o - | llvm-objdump -d - | FileCheck %s -check-prefix DUMP
#
# Check that the assembler is able to handle capability get instructions.
#

# DUMP-LABEL: Disassembly of section .text:
# DUMP-NEXT: .text:

# CHECK: cgetperm	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x08,0x3f]
# DUMP-NEXT: 48 0c 08 3f 	cgetperm	$12, $c1
	cgetperm	$t0, $c1

# CHECK: cgettype	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x08,0x7f]
# DUMP-NEXT: 48 0c 08 7f 	cgettype	$12, $c1
	cgettype	$t0, $c1

# CHECK: cgetbase	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x08,0xbf]
# DUMP-NEXT: 48 0c 08 bf 	cgetbase	$12, $c1
	cgetbase	$t0, $c1

# CHECK: cgetoffset	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x09,0xbf]
# DUMP-NEXT: 48 0c 09 bf 	cgetoffset	$12, $c1
	cgetoffset	$t0, $c1

# CHECK: cgetaddr	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x0b,0xff]
# DUMP-NEXT: 48 0c 0b ff 	cgetaddr	$12, $c1
	cgetaddr	$t0, $c1

# CHECK: cgetlen	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x08,0xff]
# DUMP-NEXT: 48 0c 08 ff 	cgetlen	$12, $c1
	cgetlen		$t0, $c1

# CHECK: cgettag	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x09,0x3f]
# DUMP-NEXT: 48 0c 09 3f 	cgettag	$12, $c1
	cgettag		$t0, $c1

# CHECK: cgetsealed	$12, $c1
# CHECK-SAME: encoding: [0x48,0x0c,0x09,0x7f]
# DUMP-NEXT: 48 0c 09 7f 	cgetsealed	$12, $c1
	cgetsealed	$t0, $c1

# CHECK: cgetpcc	$c1
# CHECK-SAME: encoding: [0x48,0x01,0x07,0xff]
# DUMP-NEXT: 48 01 07 ff 	cgetpcc	$c1
	cgetpcc	$c1

# CHECK: cgetpccsetoffset	$c1, $3
# CHECK-SAME: encoding: [0x48,0x01,0x19,0xff]
# DUMP-NEXT: 48 01 19 ff 	cgetpccsetoffset	$c1, $3
	cgetpccsetoffset		$c1, $3

# Check that this doesn't give a CGetPCC
# CHECK: cgetpccsetoffset	$c1, $zero
# DUMP-NEXT: 48 01 01 ff 	cgetpccsetoffset	$c1, $zero
# CHECK-SAME: encoding: [0x48,0x01,0x01,0xff]
	cgetpccsetoffset		$c1, $zero

# CHECK: cgetcause	$12
# CHECK-SAME: encoding: [0x48,0x0c,0x0f,0xff]
# DUMP-NEXT: 48 0c 0f ff 	cgetcause	$12
	cgetcause	$t0

# CHECK: cgetepcc	 $c4
# CHECK-SAME:  encoding: [0x48,0x04,0xf8,0x11]
# DUMP-NEXT: 48 04 f8 11 	cgetepcc	$c4
	CGetEPCC $c4

# CHECK: csetepcc	 $c4
# CHECK-SAME:  encoding: [0x48,0x1f,0x20,0x11]
# DUMP-NEXT: 48 1f 20 11 	csetepcc	$c4
	CSetEPCC $c4

# CHECK: cgetkcc	 $c4
# CHECK-SAME:  encoding: [0x48,0x04,0xe8,0x11]
# DUMP-NEXT: 48 04 e8 11 	cgetkcc	$c4
	CGetKCC $c4

# CHECK: csetkcc	 $c4
# CHECK-SAME:  encoding: [0x48,0x1d,0x20,0x11]
# DUMP-NEXT: 48 1d 20 11 	csetkcc	$c4
	CSetKCC $c4

# CHECK: cgetkdc	 $c4
# CHECK-SAME:  encoding: [0x48,0x04,0xf0,0x11]
# DUMP-NEXT: 48 04 f0 11 	cgetkdc	$c4
	CGetKDC $c4

# CHECK: csetkdc	 $c4
# CHECK-SAME:  encoding: [0x48,0x1e,0x20,0x11]
# DUMP-NEXT: 48 1e 20 11 	csetkdc	$c4
	CSetKDC $c4

# CHECK: cgetkr1c	 $c4
# CHECK-SAME:  encoding: [0x48,0x04,0xd8,0x11]
# DUMP-NEXT: 48 04 d8 11 	cgetkr1c	$c4
	CGetKR1C $c4

# CHECK: csetkr1c	 $c4
# CHECK-SAME:  encoding: [0x48,0x1b,0x20,0x11]
# DUMP-NEXT: 48 1b 20 11 	csetkr1c	$c4
	CSetKR1C $c4

# CHECK: cgetkr2c	 $c4
# CHECK-SAME:  encoding: [0x48,0x04,0xe0,0x11]
# DUMP-NEXT: 48 04 e0 11 	cgetkr2c	$c4
	CGetKR2C $c4

# CHECK: csetkr2c	 $c4
# CHECK-SAME:  encoding: [0x48,0x1c,0x20,0x11]
# DUMP-NEXT: 48 1c 20 11 	csetkr2c	$c4
	CSetKR2C $c4

# CHECK: csub	 $1, $c2, $c3
# CHECK-SAME:  encoding: [0x48,0x01,0x10,0xca]
# DUMP-NEXT: 48 01 10 ca 	csub	$1, $c2, $c3
	csub	$1, $c2, $c3


# Check that cgetnull $cN is an alias for cfromptr $cN, $c0, $zero:
# CHECK:      cgetnull  $c2
# CHECK-SAME: encoding: [0x48,0x02,0x00,0x13]
	cfromptr	$c2, $c0, $zero
# CHECK:      cgetnull  $c1
# CHECK-SAME: encoding: [0x48,0x01,0x00,0x13]
	cgetnull	$c1
# Both should disassemble to cgetnull:
# DUMP-NEXT: 48 02 00 13 	cgetnull	$c2
# DUMP-NEXT: 48 01 00 13 	cgetnull	$c1

# CHECK:      cfromddc  $c3, $4
# CHECK-SAME: encoding: [0x48,0x03,0x01,0x13]
# DUMP-NEXT: 48 03 01 13 cfromddc $c3, $4
	cfromddc	$c3, $4
	# Check that cfromddc	$c4, $zero disassembles to cgetnull
# CHECK:      cgetnull	$c4
# CHECK-SAME: encoding: [0x48,0x04,0x00,0x13]
# DUMP-NEXT: 48 04 00 13 cgetnull $c4
	cfromddc	$c4, $zero
