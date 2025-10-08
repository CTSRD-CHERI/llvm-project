# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/merge_section_got.so
# RUN: llvm-readelf -t %t/merge_section_got.so | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-objdump -d --no-show-raw-insn %t/merge_section_got.so | FileCheck --check-prefix=DIS %s
# RUN: llvm-readelf --cap-relocs %t/merge_section_got.so | FileCheck --check-prefix=CAP-RELOCS %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:         [10] .rodata.one
# SECTIONS-NEXT:         PROGBITS        0000000000003598 000598 000022 01   0   0  8
# SECTIONS-NEXT:         [0000000000000032]: ALLOC, MERGE, STRINGS
# SECTIONS-NEXT:    [11] .text.one
# SECTIONS-NEXT:         PROGBITS        00000000000045bc 0005bc 00000c 00   0   0  4
# SECTIONS-NEXT:         [0000000000000006]: ALLOC, EXEC
# SECTIONS-NEXT:    [12] .pad.cheri.pcc.one
# SECTIONS-NEXT:         PROGBITS        00000000000045c8 0005c8 000000 00   0   0  1
# SECTIONS-NEXT:         [0000000000000006]: ALLOC, EXEC
# SECTIONS-NEXT:    [13] .rodata.two
# SECTIONS-NEXT:         PROGBITS        00000000000055d0 0005d0 000022 01   0   0  16
# SECTIONS-NEXT:         [0000000000000032]: ALLOC, MERGE, STRINGS
# SECTIONS-NEXT:    [14] .text.two
# SECTIONS-NEXT:         PROGBITS        00000000000065f4 0005f4 00000c 00   0   0  4
# SECTIONS-NEXT:         [0000000000000006]: ALLOC, EXEC
# SECTIONS-NEXT:    [15] .got.two
# SECTIONS-NEXT:         PROGBITS        0000000000007600 000600 000020 00   0   0  16
# SECTIONS-NEXT:         [0000000000000003]: WRITE, ALLOC
# SECTIONS-NEXT:    [16] .pad.cheri.pcc.two

# DIS-LABEL: Disassembly of section .text.one:
# DIS:       <foo>:
## .Lfoo_str.one - . = 0x3598 - 0x45bc = 4096*-1 - 24
# DIS-NEXT:  45bc:       auipcc  ct0, 1048575
# DIS-NEXT:              cincoffset      ct0, ct0, -36

# DIS-LABEL: Disassembly of section .text.two:
# DIS:       <bar>:
## .Lbar_str.two@got - . = 0x7610 - 0x65f4 = 4096*1 + 28
# DIS-NEXT:  65f4:       auipcc  ct0, 1
# DIS-NEXT:              lc      ct0, 28(ct0)

# CAP-RELOCS:     Offset             Info         Type        Value
# CAP-RELOCS: 0000000000007610  4000000000000000 RODATA  00000000000055e0 [00000000000055e0-00000000000055f2]

#--- one.s

	.section .text.foo,"ax",@progbits
	.p2align 2
	.global	foo
	.type	foo, @function
foo:
	cllc	ct0, .Lfoo_str
	ret
	.size	foo, . - foo

	.section .rodata.str1.1,"aMS",@progbits,1
.Lfoo_str:
	.asciz	"foo like grapes"
	.size	.Lfoo_str, . - .Lfoo_str

	.section .text.bar,"ax",@progbits
	.p2align 2
	.global	bar
	.type	bar, @function
bar:
	clgc	ct0, .Lbar_str
	ret
	.size	bar, . - bar

	.section .rodata.str1.1,"aMS",@progbits,1
.Lbar_str:
	.asciz	"bar likes oranges"
	.size	.Lbar_str, . - .Lbar_str

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
