# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/text-only.s -o %t/text-only.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/text-rodata.s -o %t/text-rodata.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/section-symbol.s -o %t/section-symbol.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/merge-section.s -o %t/merge-section.o
# RUN: ld.lld --shared %t/text-only.o -o %t/text-only.so
# RUN: ld.lld --shared %t/text-rodata.o -o %t/text-rodata.so
# RUN: ld.lld --static %t/section-symbol.o -o %t/section-symbol.exe
# RUN: ld.lld --shared %t/merge-section.o -o %t/merge-section.so

# RUN: llvm-readelf -S -l %t/text-only.so | FileCheck --check-prefix=TEXT-ONLY %s
# RUN: llvm-readelf -S -l %t/text-rodata.so | FileCheck --check-prefix=TEXT-RODATA %s
# RUN: llvm-readelf -S -l %t/section-symbol.exe | FileCheck --check-prefix=SECTION-SYMBOL %s
# RUN: llvm-readelf -S -l %t/merge-section.so | FileCheck --check-prefix=MERGE-SECTION %s

## text-only's segment should only cover .text
# TEXT-ONLY-LABEL: Section Headers:
# TEXT-ONLY:         [ 6] .text             PROGBITS        0000000000003270 002270 000004 00  AX  0   0  4
# TEXT-ONLY-LABEL: Program Headers:
# TEXT-ONLY:         CHERI_PCC      0x000000 0x0000000000003270 0x0000000000003270 0x000000 0x000004 R E 0x1

## text-rodata's segment should cover .rodata as well
# TEXT-RODATA-LABEL: Section Headers:
# TEXT-RODATA:         [ 5] .rodata           PROGBITS        0000000000000270 000270 002000 00   A  0   0 16
# TEXT-RODATA-NEXT:    [ 6] .text             PROGBITS        0000000000003270 002270 00000c 00  AX  0   0  4
# TEXT-RODATA-NEXT:    [ 7] .pad.cheri.pcc    PROGBITS        000000000000327c 00227c 000004 00  AX  0   0  1
# TEXT-RODATA-LABEL: Program Headers:
# TEXT-RODATA:         CHERI_PCC      0x000000 0x0000000000000270 0x0000000000000270 0x000000 0x003010 R E 0x10

## section-symbol's segment should cover __cap_relocs as well
# SECTION-SYMBOL-LABEL: Section Headers:
# SECTION-SYMBOL:         [ 1] __cap_relocs      PROGBITS        0000000000010190 000190 000028 28   A  0   0  8
# SECTION-SYMBOL-NEXT:    [ 2] .text             PROGBITS        00000000000111b8 0001b8 00000c 00  AX  0   0  4
# SECTION-SYMBOL-NEXT:    [ 3] .pad.cheri.pcc    PROGBITS        00000000000111c4 0001c4 000004 00  AX  0   0  1
# SECTION-SYMBOL-LABEL: Program Headers:
# SECTION-SYMBOL:         CHERI_PCC      0x000000 0x0000000000010190 0x0000000000010190 0x000000 0x001038 R E 0x8

## merge-section's segment should cover .merge as well
# MERGE-SECTION-LABEL: Section Headers:
# MERGE-SECTION:         [ 5] .merge            PROGBITS        0000000000000270 000270 000004 04  AM  0   0  8
# MERGE-SECTION-NEXT:    [ 6] .text             PROGBITS        0000000000001274 000274 00000c 00  AX  0   0  4
# MERGE-SECTION-LABEL: Program Headers:
# MERGE-SECTION:         CHERI_PCC      0x000000 0x0000000000000270 0x0000000000000270 0x000000 0x001010 R E 0x8

#--- text-only.s

	.global foo
	.type foo, @function
foo:
	ret
	.size	foo, . - foo

	.rodata
	.type bar, @object
bar:
	.space	8192
	.size	bar, . - bar

#--- text-rodata.s

	.global foo
	.type	foo, @function
foo:
	cllc	ct0, bar
	ret
	.size	foo, . - foo

	.rodata
	.type	bar, @object
bar:
	.space	8192
	.size	bar, . - bar

#--- section-symbol.s

	.global foo
	.type	foo, @function
foo:
	cllc	ct0, __start___cap_relocs
	ret
	.size	foo, . - foo

	.data
	.global	bar
	.type	bar, @object
bar:
	.chericap foo
	.size	bar, . - bar

#--- merge-section.s

	.global foo
	.type	foo, @function
foo:
	cllc	ct0, bar
	ret
	.size	foo, . - foo

	.section .merge,"aM",@progbits,4
	.p2align 2
bar:
	.long	1
