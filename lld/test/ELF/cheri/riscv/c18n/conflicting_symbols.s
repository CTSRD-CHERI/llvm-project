# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: not ld.lld --shared --verbose-c18n --compartment-policy=%t/compartments.json %t/one.o -o %t/conflicting_symbols.so.0 2>&1 | FileCheck %s

# CHECK: one.o:.textassigned to compartment one by symbol foo and compartment two by symbol bar

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

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
