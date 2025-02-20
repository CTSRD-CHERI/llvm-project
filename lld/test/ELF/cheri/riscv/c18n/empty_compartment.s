# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/empty.so.0 2>&1 | FileCheck %s

# CHECK: ld.lld: error: no output sections for compartment two

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
