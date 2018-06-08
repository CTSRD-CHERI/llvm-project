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

.macro cheri_opcode_3arg code, r1, r2, r3
  .word ((0x12 << 26) | (\r1 << 16) | (\r2 << 11) | (\r3 << 6) | \code)
.endm
.macro cheri_opcode_2arg code, r1, r2
  cheri_opcode_3arg 0x3f, \r1, \r2, \code
.endm
.macro cheri_opcode_1arg code, r1
  cheri_opcode_3arg 0x3f, \r1, \code, 0x3f
.endm

# Allow generating the old cmove even if the registers are no longer used
.macro old_cget_c4 regno
  # cmove $c4 $regno
  # cheri_opcode_2arg 0xa, 4, \regno
  # use cincoffset instead
  cheri_opcode_3arg 0x11, 4, \regno, 0
.endm
.macro old_cset_c4 regno
  # cmove $regno, c4
  # cheri_opcode_2arg 0xa, \regno, 4
  # use cincoffset instead
  cheri_opcode_3arg 0x11, \regno, 4, 0
.endm

# CHECK: creadhwr	 $c4, $chwr_epcc
# CHECK-SAME:  encoding: [0x48,0x04,0xfb,0x7f]
# DUMP-NEXT: 48 04 fb 7f 	creadhwr	 $c4, $chwr_epcc
	CGetEPCC $c4

# CHECK: cwritehwr	 $c4, $chwr_epcc
# CHECK-SAME:  encoding: [0x48,0x04,0xfb,0xbf]
# DUMP-NEXT: 48 04 fb bf 	cwritehwr	 $c4, $chwr_epcc
	CSetEPCC $c4

	# test disassembly of the old encoding:
	old_cget_c4 31
	# DUMP-NEXT: 48 04 f8 11 cincoffset $c4, $c31, $zero
	old_cset_c4 31
	# DUMP-NEXT: 48 1f 20 11 cincoffset $c31, $c4, $zero

# CHECK: creadhwr        $c4, $chwr_kcc
# CHECK-SAME:  encoding: [0x48,0x04,0xeb,0x7f]
# DUMP-NEXT: 48 04 eb 7f     creadhwr        $c4, $chwr_kcc
	CGetKCC $c4

# CHECK: cwritehwr        $c4, $chwr_kcc
# CHECK-SAME:  encoding: [0x48,0x04,0xeb,0xbf]
# DUMP-NEXT: 48 04 eb bf     cwritehwr       $c4, $chwr_kcc
	CSetKCC $c4

	# test disassembly of the old encoding:
	old_cget_c4 29
	# DUMP-NEXT: 48 04 e8 11 cincoffset $c4, $c29, $zero
	old_cset_c4 29
	# DUMP-NEXT: 48 1d 20 11 cincoffset $c29, $c4, $zero

# CHECK: creadhwr        $c4, $chwr_kdc
# CHECK-SAME:  encoding: [0x48,0x04,0xf3,0x7f]
# DUMP-NEXT: 48 04 f3 7f     creadhwr        $c4, $chwr_kdc
	CGetKDC $c4

# CHECK: cwritehwr        $c4, $chwr_kdc
# CHECK-SAME:  encoding: [0x48,0x04,0xf3,0xbf]
# DUMP-NEXT: 48 04 f3 bf     cwritehwr       $c4, $chwr_kdc
	CSetKDC $c4

	# test disassembly of the old encoding:
	old_cget_c4 30
	# DUMP-NEXT: 48 04 f0 11 cincoffset $c4, $c30, $zero
	old_cset_c4 30
	# DUMP-NEXT: 48 1e 20 11 cincoffset $c30, $c4, $zero


# CHECK: creadhwr	 $c4, $chwr_kr1c
# CHECK-SAME:  encoding: [0x48,0x04,0xb3,0x7f]
# DUMP-NEXT: 48 04 b3 7f 	creadhwr	 $c4, $chwr_kr1c
	CGetKR1C $c4

# CHECK: cwritehwr	 $c4, $chwr_kr1c
# CHECK-SAME:  encoding: [0x48,0x04,0xb3,0xbf]
# DUMP-NEXT: 48 04 b3 bf 	cwritehwr	 $c4, $chwr_kr1c
	CSetKR1C $c4

# test disassembly of the old encoding:
old_cget_c4 27
# DUMP-NEXT: 48 04 d8 11 cincoffset $c4, $c27, $zero
old_cset_c4 27
# DUMP-NEXT: 48 1b 20 11 cincoffset $c27, $c4, $zero

# CHECK: creadhwr	 $c4, $chwr_kr2c
# CHECK-SAME:  encoding: [0x48,0x04,0xbb,0x7f]
# DUMP-NEXT: 48 04 bb 7f 	creadhwr	 $c4, $chwr_kr2c
	CGetKR2C $c4

# CHECK: cwritehwr	 $c4, $chwr_kr2c
# CHECK-SAME:  encoding: [0x48,0x04,0xbb,0xbf]
# DUMP-NEXT: 48 04 bb bf 	cwritehwr	 $c4, $chwr_kr2c
	CSetKR2C $c4
# test disassembly of the old encoding:
old_cget_c4 28
# DUMP-NEXT: 48 04 e0 11 cincoffset $c4, $c28, $zero
old_cset_c4 28
# DUMP-NEXT: 48 1c 20 11 cincoffset $c28, $c4, $zero



# CHECK:  creadhwr        $c4, $chwr_ddc
# CHECK-SAME:  encoding: [0x48,0x04,0x03,0x7f]
# DUMP-NEXT: 48 04 03 7f creadhwr        $c4, $chwr_ddc
	CGetDefault $c4

# CHECK:  cwritehwr       $c4, $chwr_ddc
# CHECK-SAME:  encoding: [0x48,0x04,0x03,0xbf]
# DUMP-NEXT: 48 04 03 bf cwritehwr       $c4, $chwr_ddc
	CSetDefault $c4

	# test disassembly of the old encoding:
	old_cget_c4 0
	# DUMP-NEXT: 48 04 00 11 cincoffset $c4, $cnull, $zero
	old_cset_c4 0
	# DUMP-NEXT: 48 00 20 11 cincoffset $cnull, $c4, $zero

# CHECK: csub	 $1, $c2, $c3
# CHECK-SAME:  encoding: [0x48,0x01,0x10,0xca]
# DUMP-NEXT: 48 01 10 ca 	csub	$1, $c2, $c3
	csub	$1, $c2, $c3


# Check that cgetnull $cN is an alias for cfromptr $cN, $c0, $zero:
# CHECK:      cgetnull  $c2
# CHECK-SAME: encoding: [0x48,0x02,0x00,0x13]
	cfromptr	$c2, $ddc, $zero
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
