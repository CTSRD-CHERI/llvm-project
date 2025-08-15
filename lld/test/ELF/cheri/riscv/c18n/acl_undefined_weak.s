# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_undefined_weak.exe

#--- one.s

	.weak	_DYNAMIC

	.text
	.global	foo
	.type	foo, @function
foo:
	clgc	ct0, _DYNAMIC
	ret
	.size	foo, . - foo

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"r", "compartments":[""] }
    ]
}
