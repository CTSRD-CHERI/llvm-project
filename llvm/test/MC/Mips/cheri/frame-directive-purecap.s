# RUN: %cheri_llvm-mc %s -show-encoding

# Check that we accept $c17 in the .frame directive return register
# This is what clang emits so we should be able to parse it...

# This is what clang emits for `int foo() { return 1; } with -mabi=purecap and -fomit-frame-pointer
	.text
	.abicalls
	.section	.mdebug.abi64,"",@progbits
	.nan	legacy
	.file	"foo.c"
	.text
	.globl	foo                     # -- Begin function foo
	.p2align	3
	.type	foo,@function
	.set	nomicromips
	.set	nomips16
	.ent	foo
foo:                                    # @foo
	.frame	$c11,0,$c17
	.mask 	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
# %bb.0:                                # %entry
	addiu	$2, $zero, 1
	cjr	$c17
	nop
	.set	at
	.set	macro
	.set	reorder
	.end	foo
.Lfunc_end0:
	.size	foo, .Lfunc_end0-foo
