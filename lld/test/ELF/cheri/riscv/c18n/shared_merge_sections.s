# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/shared_merge_sections.so
# COM: readelf -t %t/shared_merge_sections.so | FileCheck %s

#--- one.s

	.section .text.foo,"ax",@progbits
	.global	foo
	.type	foo, @function
foo:
	cllc	ct0, .Lfoo_data
	ret
	.size	foo, . - foo

	.section .rodata.cst4,"aM",@progbits,4
	.p2align 2
.Lfoo_data:
	.word	0x1234
	.size	.Lfoo_data, . - .Lfoo_data

	.section .text.bar,"ax",@progbits
	.global	bar
	.type	bar, @function
bar:
	cllc	ct0, .Lbar_data
	ret
	.size	bar, . - bar

	.section .rodata.cst4,"aM",@progbits,4
	.p2align 2
.Lbar_data:
	.word	0x5678
	.size	.Lbar_data, . - .Lbar_data

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
