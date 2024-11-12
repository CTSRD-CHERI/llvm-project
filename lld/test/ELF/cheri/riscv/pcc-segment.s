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

# RUN: llvm-readelf -l %t/text-only.so | FileCheck --check-prefix=TEXT-ONLY %s
# RUN: llvm-readelf -l %t/text-rodata.so | FileCheck --check-prefix=TEXT-RODATA %s
# RUN: llvm-readelf -l %t/section-symbol.exe | FileCheck --check-prefix=SECTION-SYMBOL %s
# RUN: llvm-readelf -l %t/merge-section.so | FileCheck --check-prefix=MERGE-SECTION %s

## text-only's segment will eventually only cover .text, but .rodata is
## included for now
# TEXT-ONLY-LABEL: Section to Segment mapping:
# TEXT-ONLY:         06     .rodata .text .pad.cheri.pcc {{$}}

## text-rodata's segment should cover .rodata as well
# TEXT-RODATA-LABEL: Section to Segment mapping:
# TEXT-RODATA:         06     .rodata .text .pad.cheri.pcc {{$}}

## section-symbol's segment should cover cap_relocs as well
# SECTION-SYMBOL-LABEL: Section to Segment mapping:
# SECTION-SYMBOL:         04     __cap_relocs .text .pad.cheri.pcc {{$}}

## merge-section's segment only covers .text
# MERGE-SECTION-LABEL: Section to Segment mapping:
# MERGE-SECTION:         06     .merge .text {{$}}

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
