# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/small_text.s -o %t/small_text.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/big_text.s -o %t/big_text.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/text_rodata.s -o %t/text_rodata.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/rodata_only.s -o %t/rodata_only.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/plt.s -o %t/plt.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/plt_notext.s -o %t/plt_notext.o
# RUN: ld.lld --shared %t/small_text.o -o %t/small_text.so
# RUN: ld.lld --shared %t/big_text.o -o %t/big_text.so
# RUN: ld.lld --shared %t/text_rodata.o -o %t/text_rodata.so
# RUN: ld.lld --shared %t/rodata_only.o -o %t/rodata_only.so
# RUN: ld.lld --shared %t/plt.o -o %t/plt.so
# RUN: ld.lld --shared %t/plt_notext.o -o %t/plt_notext.so

# RUN: llvm-readelf -t %t/small_text.so | FileCheck --check-prefix=SMALL_TEXT %s
# RUN: llvm-readelf -t %t/big_text.so | FileCheck --check-prefix=BIG_TEXT %s
# RUN: llvm-readelf -t %t/text_rodata.so | FileCheck --check-prefix=TEXT_RODATA %s
# RUN: llvm-readelf -t %t/rodata_only.so | FileCheck --check-prefix=RODATA_ONLY %s
# RUN: llvm-readelf -t %t/plt.so | FileCheck --check-prefix=PLT %s
# RUN: llvm-readelf -t %t/plt_notext.so | FileCheck --check-prefix=PLT_NOTEXT %s

# small_text results in zero byte padding section :(
# SMALL_TEXT-LABEL: Section Headers:
# SMALL_TEXT:         [ 5] .text
# SMALL_TEXT-NEXT:         PROGBITS        0000000000001270 000270 000004 00   0   0  4
# SMALL_TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# SMALL_TEXT-NEXT:    [ 6] .pad.cheri.pcc
# SMALL_TEXT-NEXT:         PROGBITS        0000000000001274 000274 000000 00   0   0  1
# SMALL_TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# SMALL_TEXT-NEXT:    [ 7] .dynamic
# SMALL_TEXT-NEXT:         DYNAMIC         0000000000002278 000278 000070 10   4   0  8

# big_text results in actual padding
# BIG_TEXT-LABEL: Section Headers:
# BIG_TEXT:         [ 5] .text
# BIG_TEXT-NEXT:         PROGBITS        0000000000001270 000270 002004 00   0   0  16
# BIG_TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# BIG_TEXT-NEXT:    [ 6] .pad.cheri.pcc
# BIG_TEXT-NEXT:         PROGBITS        0000000000003274 002274 00000c 00   0   0  1
# BIG_TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# BIG_TEXT-NEXT:    [ 7] .dynamic
# BIG_TEXT-NEXT:         DYNAMIC         0000000000004280 002280 000070 10   4   0  8

# text_rodata triggers padding as well
# TEXT_RODATA-LABEL: Section Headers:
# TEXT_RODATA:         [ 5] .rodata
# TEXT_RODATA-NEXT:         PROGBITS        00000000000002a0 0002a0 002000 00   0   0  16
# TEXT_RODATA-NEXT:         [0000000000000002]: ALLOC
# TEXT_RODATA-NEXT:    [ 6] .text
# TEXT_RODATA-NEXT:         PROGBITS        00000000000032a0 0022a0 000004 00   0   0  4
# TEXT_RODATA-NEXT:         [0000000000000006]: ALLOC, EXEC
# TEXT_RODATA-NEXT:    [ 7] .pad.cheri.pcc
# TEXT_RODATA-NEXT:         PROGBITS        00000000000032a4 0022a4 00000c 00   0   0  1
# TEXT_RODATA-NEXT:         [0000000000000006]: ALLOC, EXEC
# TEXT_RODATA-NEXT:    [ 8] .dynamic
# TEXT_RODATA-NEXT:         DYNAMIC         00000000000042b0 0022b0 000070 10   4   0  8
# TEXT_RODATA-NEXT:         [0000000000000003]: WRITE, ALLOC

# rodata_only should not result in a padding section
# RODATA_ONLY-LABEL: Section Headers:
# RODATA_ONLY-NOT:     .pad.cheri.pcc

# plt uses RW for the padding to match .got.plt
# PLT-LABEL: Section Headers:
# PLT:         [11] .got.plt
# PLT-NEXT:         PROGBITS        0000000000005460 002460 000030 00   0   0  16
# PLT-NEXT:         [0000000000000003]: WRITE, ALLOC
# PLT-NEXT:    [12] .pad.cheri.pcc
# PLT-NEXT:         PROGBITS        0000000000005490 002490 000010 00   0   0  1
# PLT-NEXT:         [0000000000000003]: WRITE, ALLOC

# plt_notext still generates padding due to .iplt
# PLT_NOTEXT-LABEL: Section Headers:
# PLT_NOTEXT:         [ 7] .text
# PLT_NOTEXT-NEXT:         PROGBITS        0000000000003310 002310 000000 00   0   0  4
# PLT_NOTEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# PLT_NOTEXT-NEXT:    [ 8] .iplt
# PLT_NOTEXT-NEXT:         PROGBITS        0000000000003310 002310 000010 00   0   0  16
# PLT_NOTEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# PLT_NOTEXT:         [11] .got.plt
# PLT_NOTEXT-NEXT:         PROGBITS        00000000000053c0 0023c0 000010 00   0   0  16
# PLT_NOTEXT-NEXT:         [0000000000000003]: WRITE, ALLOC
# PLT_NOTEXT-NEXT:    [12] .pad.cheri.pcc
# PLT_NOTEXT-NEXT:         PROGBITS        00000000000053d0 0023d0 000010 00   0   0  1
# PLT_NOTEXT-NEXT:         [0000000000000003]: WRITE, ALLOC

#--- small_text.s

	.global foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- big_text.s

	.global foo
	.type	foo, @function
foo:
	ret
	.space	8192
	.size	foo, . - foo

#--- text_rodata.s

	.global foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

	.rodata
	.global	bar
	.type	bar, @object
bar:
	.space	8192
	.size	bar, . - bar

#--- rodata_only.s

	.rodata
	.global	bar
	.type	bar, @object
bar:
	.space	8196
	.size	bar, . - bar

#--- plt.s

	.global foo
	.type	foo, @function
foo:
	ccall	baz@plt
	ret
	.size	foo, . - foo

	.rodata
	.global	bar
	.type	bar, @object
bar:
	.space	8192
	.size	bar, . - bar

#--- plt_notext.s

# Using an empty function is a gross way of simulating an empty
# text section to ensure that padding is inserted if the only
# executable sections are synthetic sections.

	.text
	.global foo
	.type	foo, STT_GNU_IFUNC
	.hidden	foo
foo:
	.size	foo, 0

	.data
	.type	ptr, @object
ptr:	.chericap foo
	.size	ptr, . - ptr

	.rodata
	.global	bar
	.type	bar, @object
bar:
	.space	8192
	.size	bar, . - bar
