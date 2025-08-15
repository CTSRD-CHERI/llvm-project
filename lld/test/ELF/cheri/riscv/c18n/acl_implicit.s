# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_implicit.so
# RUN: readelf -r %t/acl_implicit.so | FileCheck %s

# `bar` symbol implicitly assigned to compartment one can be accessed
# without an explicit ACL.

# CHECK-LABEL: Relocation section '.rela.plt.one'
# CHECK:       0000000000004560  0000000300000005 R_RISCV_JUMP_SLOT      00000000000034f4 bar + 0
# CHECK:       0000000000004570  0000000100000005 R_RISCV_JUMP_SLOT      0000000000000000 printf + 0

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ccall	bar
	ccall	printf@plt
	ret
	.size	foo, . - foo

	.text
	.global bar
	.type	bar, @function
bar:
	ret
	.size	bar, . - bar

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"x", "symbols":["printf"] }
    ]
}
