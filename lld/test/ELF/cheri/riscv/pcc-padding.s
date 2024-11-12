# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/small-text.s -o %t/small-text.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/big-text.s -o %t/big-text.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/text-rodata.s -o %t/text-rodata.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/rodata-only.s -o %t/rodata-only.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/plt.s -o %t/plt.o
# RUN: ld.lld --shared %t/small-text.o -o %t/small-text.so
# RUN: ld.lld --shared %t/big-text.o -o %t/big-text.so
# RUN: ld.lld --shared %t/text-rodata.o -o %t/text-rodata.so
# RUN: ld.lld --shared %t/rodata-only.o -o %t/rodata-only.so
# RUN: ld.lld --shared %t/plt.o -o %t/plt.so

# RUN: llvm-readelf -t %t/small-text.so | FileCheck --check-prefix=SMALL-TEXT %s
# RUN: llvm-readelf -t %t/big-text.so | FileCheck --check-prefix=BIG-TEXT %s
# RUN: llvm-readelf -t %t/text-rodata.so | FileCheck --check-prefix=TEXT-RODATA %s
# RUN: llvm-readelf -t %t/rodata-only.so | FileCheck --check-prefix=RODATA-ONLY %s
# RUN: llvm-readelf -t %t/plt.so | FileCheck --check-prefix=PLT %s

# small-text results in zero byte padding section :(
# SMALL-TEXT-LABEL: Section Headers:
# SMALL-TEXT:         [ 5] .text
# SMALL-TEXT-NEXT:         PROGBITS        0000000000001270 000270 000004 00   0   0  4
# SMALL-TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# SMALL-TEXT-NEXT:    [ 6] .pad.cheri.pcc
# SMALL-TEXT-NEXT:         PROGBITS        0000000000001274 000274 000000 00   0   0  1
# SMALL-TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# SMALL-TEXT-NEXT:    [ 7] .dynamic
# SMALL-TEXT-NEXT:         DYNAMIC         0000000000002278 000278 000070 10   4   0  8

# big-text results in actual padding
# BIG-TEXT-LABEL: Section Headers:
# BIG-TEXT:         [ 5] .text
# BIG-TEXT-NEXT:         PROGBITS        0000000000001270 000270 002004 00   0   0  16
# BIG-TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# BIG-TEXT-NEXT:    [ 6] .pad.cheri.pcc
# BIG-TEXT-NEXT:         PROGBITS        0000000000003274 002274 00000c 00   0   0  1
# BIG-TEXT-NEXT:         [0000000000000006]: ALLOC, EXEC
# BIG-TEXT-NEXT:    [ 7] .dynamic
# BIG-TEXT-NEXT:         DYNAMIC         0000000000004280 002280 000070 10   4   0  8

# text-rodata triggers padding as well
# TEXT-RODATA-LABEL: Section Headers:
# TEXT-RODATA:         [ 5] .rodata
# TEXT-RODATA-NEXT:         PROGBITS        0000000000000270 000270 002000 00   0   0  16
# TEXT-RODATA-NEXT:         [0000000000000002]: ALLOC
# TEXT-RODATA-NEXT:    [ 6] .text
# TEXT-RODATA-NEXT:         PROGBITS        0000000000003270 002270 00000c 00   0   0  4
# TEXT-RODATA-NEXT:         [0000000000000006]: ALLOC, EXEC
# TEXT-RODATA-NEXT:    [ 7] .pad.cheri.pcc
# TEXT-RODATA-NEXT:         PROGBITS        000000000000327c 00227c 000004 00   0   0  1
# TEXT-RODATA-NEXT:         [0000000000000006]: ALLOC, EXEC
# TEXT-RODATA-NEXT:    [ 8] .dynamic
# TEXT-RODATA-NEXT:         DYNAMIC         0000000000004280 002280 000070 10   4   0  8
# TEXT-RODATA-NEXT:         [0000000000000003]: WRITE, ALLOC

# rodata-only should not result in a padding section
# RODATA-ONLY-LABEL: Section Headers:
# RODATA-ONLY-NOT:     .pad.cheri.pcc

# plt uses RW for the padding to match .got.plt
# PLT-LABEL: Section Headers:
# PLT:         [11] .got.plt
# PLT-NEXT:         PROGBITS        0000000000005440 002440 000030 00   0   0  16
# PLT-NEXT:         [0000000000000003]: WRITE, ALLOC
# PLT-NEXT:    [12] .pad.cheri.pcc
# PLT-NEXT:         PROGBITS        0000000000005470 002470 000010 00   0   0  1
# PLT-NEXT:         [0000000000000003]: WRITE, ALLOC

#--- small-text.s

	.global foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- big-text.s

	.global foo
	.type	foo, @function
foo:
	ret
	.space	8192
	.size	foo, . - foo

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

#--- rodata-only.s

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
	cllc	ct0, bar
	ccall	baz@plt
	ret
	.size	foo, . - foo

	.rodata
	.type	bar, @object
bar:
	.space	8192
	.size	bar, . - bar
