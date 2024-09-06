# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: not ld.lld --shared --no-default-compartment %t/one.o %t/two.o -o %t/no_default.so.0 2>&1 | FileCheck -check-prefixes=ONE,TWO %s
# RUN: not ld.lld --shared --compartment-policy=%t/empty.json --no-default-compartment %t/one.o %t/two.o -o %t/no_default.so.0 2>&1 | FileCheck -check-prefixes=ONE,TWO %s
# RUN: not ld.lld --shared --compartment-policy=%t/one.json --no-default-compartment %t/one.o %t/two.o -o %t/no_default.so.0 2>&1 | FileCheck -check-prefix=TWO %s
# RUN: ld.lld --shared --compartment-policy=%t/both.json --no-default-compartment %t/one.o %t/two.o -o %t/no_default.so.0

# ONE: ld.lld: error: input section {{.*}}one.o:.text not assigned to a compartment
# TWO: ld.lld: error: input section {{.*}}two.o:.text not assigned to a compartment

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- two.s

	.text
	.global	bar
	.type	bar, @function
bar:
	ret
	.size	bar, . - bar

#--- empty.json

{
    "compartments": {
    }
}

#--- one.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    }
}

#--- both.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
