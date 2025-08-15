# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_noaccess.so 2>&1 | FileCheck %s

# CHECK-DAG: error: policy forbids accessing symbol baz from compartment one
# CHECK-DAG: error: policy forbids executing symbol gets from compartment one

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ccall	bar
	clgc	ct0, baz
	ccall	printf@plt
	ccall	gets@plt
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
	"one": { "symbols": ["foo", "bar"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"x", "symbols":["printf"] }
    ]
}
