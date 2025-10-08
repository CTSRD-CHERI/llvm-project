# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/merge_section_symbol.so
# RUN: readelf -t %t/merge_section_symbol.so | FileCheck --check-prefix=SECTIONS %s
# RUN: objdump -d --no-show-raw-insn %t/merge_section_symbol.so | FileCheck --check-prefix=DIS %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:         [ 9] .rodata.one
# SECTIONS-NEXT:         PROGBITS        00000000000034c0 0004c0 000008 04   0   0  8
# SECTIONS-NEXT:         [0000000000000012]: ALLOC, MERGE
# SECTIONS:         [12] .rodata.two
# SECTIONS-NEXT:         PROGBITS        00000000000054d8 0004d8 000008 04   0   0  8
# SECTIONS-NEXT:         [0000000000000012]: ALLOC, MERGE

# DIS-LABEL: Disassembly of section .text.one:
# DIS:       <foo>:
## .Lfoo_data.one - . = 0x34c0 - 0x44c8 = 4096*-1 - 8
# DIS-NEXT:  44c8:       auipcc  ct0, 1048575
# DIS-NEXT:              cincoffset      ct0, ct0, -8

# DIS-LABEL: Disassembly of section .text.two:
# DIS:       <bar>:
## .Lbar_data.two - . = 0x64e0 - 0x54dc = 4096*-1 - 4
# DIS-NEXT:  64e0:       auipcc  ct0, 1048575
# DIS-NEXT:              cincoffset      ct0, ct0, -4
## .rodata.two - . = 0x64e8 - 0x54d8 = 4096*-1 - 16
# DIS:       64e8:       auipcc  ct1, 1048575
# DIS-NEXT:              cincoffset      ct1, ct1, -16

#--- one.s

	.section .text.foo,"ax",@progbits
	.p2align 2
	.global	foo
	.type	foo, @function
foo:
	cllc	ct0, .Lfoo_data
	ret
	.size	foo, . - foo

	.section .rodata.cst4,"aM",@progbits,4
	.p2align 2
$d.0:
.Lfoo_data:
	.word	0x1234
	.size	.Lfoo_data, . - .Lfoo_data

	.section .text.bar,"ax",@progbits
	.p2align 2
	.global	bar
	.type	bar, @function
bar:
	cllc	ct0, .Lbar_data
	cllc	ct1, .rodata.cst4
	ret
	.size	bar, . - bar

	.section .rodata.cst4,"aM",@progbits,4
	.p2align 2
$d.1:
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
