# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o %t/two.o -o %t/acl_require_exec.so 2>&1 | FileCheck %s

# CHECK-DAG: error: policy forbids executing symbol foo from compartment two
# CHECK-DAG: error: policy forbids executing symbol printf from compartment one

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

	.data
	.global buf
	.type	buf, @object
buf:	.space	1024
	.size	buf, . - buf

#--- two.s

	.text
	.global baz
	.type	baz, @function
baz:
	clgc	ct0, foo
	clgc	ct1, buf
	ret
	.size	baz, . - baz

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo", "bar", "buf"] },
	"two": { "symbols": ["baz"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"r", "symbols":["printf"] },
	{ "subject":"two", "permissions":"rw", "compartments":["one"] }
    ]
}
