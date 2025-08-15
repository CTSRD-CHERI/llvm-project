# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/onebad.s -o %t/onebad.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/onebad.o -o %t/acl_default.so 2>&1 | FileCheck --check-prefix=BAD %s
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_default.so

# BAD-DAG: error: policy forbids executing symbol bar from the default compartment
# BAD-DAG: error: policy forbids accessing symbol buf from the default compartment

#--- onebad.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ccall	bar
	ccall	printf@plt
	clgc	ct0, buf
	ret
	.size	foo, . - foo

	.data
	.global buf
	.type	buf, @object
buf:
	.space	1024
	.size	buf, . - buf

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ccall	printf@plt
	ret
	.size	foo, . - foo

	.data
	.global buf
	.type	buf, @object
buf:
	.space	1024
	.size	buf, . - buf

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["buf"] }
    },
   "acls": [
	{ "subject":"", "permissions":"x", "symbols":["printf"] }
    ]
}
