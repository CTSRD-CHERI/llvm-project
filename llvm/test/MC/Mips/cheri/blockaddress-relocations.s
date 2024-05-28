# RUN: %cheri_purecap_llvm-mc -filetype=asm -o - %s
# %s | llvm-objdump -d -r
# RUN: %cheri_purecap_llvm-mc -filetype=obj -o - %s | llvm-objdump --no-print-imm-hex -d -r - | FileCheck %s

	.text
	.globl	addrof_label_in_local   # -- Begin function addrof_label_in_local
	.p2align	3
	.type	addrof_label_in_local,@function
	.ent	addrof_label_in_local
	.set noat
	.set noreorder
addrof_label_in_local:                  # @addrof_label_in_local
# %bb.0:                                # %entry
  # Get address of label:
  # FIXME: this actually works so we should be able to generate it in selectionDAG
  lui $1, %hi(.Ltmp0 - addrof_label_in_local)
  daddiu $1, $1, %lo(.Ltmp0 - addrof_label_in_local)
  cincoffset $c1, $c12, $1

  # This is the current blockaddr generation:
	clcbi	$c1, %captab20(.Ltmp0)($c26)
	cgetpcc	$c3
	csub	$1, $c1, $c3
	cincoffset	$c1, $c3, $1
	cjr	$c1
	nop
.Ltmp0:                                 # Block address taken
.LBB0_1:                                # %label2
	addiu	$2, $zero, 2
	cjr	$c17
	nop

	.end	addrof_label_in_local
.Lfunc_end0:
	.size	addrof_label_in_local, .Lfunc_end0-addrof_label_in_local


# CHECK-LABEL: <addrof_label_in_local>:
# CHECK-NEXT: 0:	3c 01 00 00 	lui	$1, 0
# CHECK-NEXT: 4:	64 21 00 24 	daddiu	$1, $1, 36
# CHECK-NEXT: 8:	48 01 60 51 	cincoffset	$c1, $c12, $1
# CHECK-NEXT: c:	74 3a 00 00 	clcbi	$c1, 0($c26)
# CHECK-NEXT:     000000000000000c:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE	.Ltmp0
