# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o %t/two.o -o %t/invalid_pcrel.so.0 2>&1 | FileCheck  %s

# CHECK: error: symbol bar assigned to compartment two is directly accessed by compartment one

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	cllc	ct0, bar
	ret
	.size	foo, . - foo

#--- two.s

	.text
	.global	bar
	.type	bar, @function
bar:
	ret
	.size	bar, . - bar

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
