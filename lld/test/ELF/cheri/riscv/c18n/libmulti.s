# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/oneb.s -o %t/oneb.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --shared --compartment=one %t/one.o %t/oneb.o --compartment=two %t/two.o -o %t/libmulti.so.0

# RUN: llvm-readelf -t %t/libmulti.so.0 | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-readelf -l %t/libmulti.so.0 | FileCheck --check-prefix=SEGMENTS %s
# RUN: llvm-readelf -s %t/libmulti.so.0 | FileCheck --check-prefix=SYMBOLS %s
# RUN: llvm-readelf -d %t/libmulti.so.0 | FileCheck --check-prefix=DYNAMIC %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:	[11] .text.one
# SECTIONS:	[12] .got.one
# SECTIONS:	[13] .rela.plt.two
# SECTIONS:	[14] .text.two
# SECTIONS:	[15] .plt.two
# SECTIONS:	[16] .got.two
# SECTIONS:	[17] .got.plt.two

# SEGMENTS:	There are 19 program headers, starting at offset 64

# SEGMENTS-LABEL: Program Headers:
# SEGMENTS-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# SEGMENTS-NEXT:    PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x000428 0x000428 R   0x8
# SEGMENTS-NEXT:    LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x0006c0 0x0006c0 R   0x1000
# SEGMENTS-NEXT:    LOAD           0x0006c0 0x00000000000016c0 0x00000000000016c0 0x000100 0x000100 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x0007c0 0x00000000000027c0 0x00000000000027c0 0x000000 0x000004 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x0007c8 0x00000000000037c8 0x00000000000037c8 0x000040 0x000040 R E 0x1000
# SEGMENTS-NEXT:    LOAD           0x000810 0x0000000000004810 0x0000000000004810 0x000020 0x000020 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x000830 0x0000000000005830 0x0000000000005830 0x000030 0x000030 R   0x1000
# SEGMENTS-NEXT:    LOAD           0x000860 0x0000000000006860 0x0000000000006860 0x0000a0 0x0000a0 R E 0x1000
# SEGMENTS-NEXT:    LOAD           0x000900 0x0000000000007900 0x0000000000007900 0x000040 0x000040 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x000940 0x0000000000008940 0x0000000000008940 0x000040 0x000040 RW  0x1000
# SEGMENTS-NEXT:    TLS            0x0006c0 0x00000000000006c0 0x00000000000006c0 0x000000 0x000080 R   0x1
# SEGMENTS-NEXT:    DYNAMIC        0x0006c0 0x00000000000016c0 0x00000000000016c0 0x000100 0x000100 RW  0x8
# SEGMENTS-NEXT:    GNU_RELRO      0x0006c0 0x00000000000016c0 0x00000000000016c0 0x000100 0x000940 R   0x1
# SEGMENTS-NEXT:    GNU_RELRO      0x000810 0x0000000000004810 0x0000000000004810 0x000020 0x0007f0 R   0x1
# SEGMENTS-NEXT:    GNU_RELRO      0x000900 0x0000000000007900 0x0000000000007900 0x000040 0x000700 R   0x1
# SEGMENTS-NEXT:    CHERI_PCC      0x0005f0 0x00000000000005f0 0x00000000000005f0 0x000006 0x000006 R E 0x1
# SEGMENTS-NEXT:    CHERI_PCC      0x0007c8 0x00000000000037c8 0x00000000000037c8 0x000068 0x001068 R E 0x10
# SEGMENTS-NEXT:    CHERI_PCC      0x000860 0x0000000000006860 0x0000000000006860 0x000120 0x002120 R E 0x10
# SEGMENTS-NEXT:    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# SEGMENTS-LABEL: Section to Segment mapping:
# SEGMENTS-NEXT:  Segment Sections...
# SEGMENTS-NEXT:   00
# SEGMENTS-NEXT:   01     .dynsym .gnu.hash .hash .dynstr .rela.dyn .rodata __cap_relocs
# SEGMENTS-NEXT:   02     .dynamic
# SEGMENTS-NEXT:   03     .bss
# SEGMENTS-NEXT:   04     .text.one
# SEGMENTS-NEXT:   05     .got.one
# SEGMENTS-NEXT:   06     .rela.plt.two
# SEGMENTS-NEXT:   07     .text.two .plt.two
# SEGMENTS-NEXT:   08     .got.two
# SEGMENTS-NEXT:   09     .got.plt.two
# SEGMENTS-NEXT:   10     .tbss
# SEGMENTS-NEXT:   11     .dynamic
# SEGMENTS-NEXT:   12     .dynamic
# SEGMENTS-NEXT:   13     .got.one
# SEGMENTS-NEXT:   14     .got.two
# SEGMENTS-NEXT:   15     .rodata
# SEGMENTS-NEXT:   16     .text.one .got.one
# SEGMENTS-NEXT:   17     .text.two .plt.two .got.two .got.plt.two
# SEGMENTS-NEXT:   18
# SEGMENTS-NEXT:   None   .comment .symtab .shstrtab .strtab

# SYMBOLS-LABEL: Symbol table '.dynsym'
# SYMBOLS:	 3: 00000000000037c8    16 FUNC    GLOBAL DEFAULT    11 set_counter
# SYMBOLS-NEXT:  4: 00000000000037d8    24 FUNC    GLOBAL DEFAULT    11 increment_counter
# SYMBOLS-NEXT:  5: 00000000000037f0    24 FUNC    GLOBAL DEFAULT    11 decrement_counter
# SYMBOLS-NEXT:  6: 0000000000006860    84 FUNC    GLOBAL DEFAULT    14 counter_str

# SYMBOLS-LABEL: Symbol table '.symtab'
# SYMBOLS:	 3: 00000000000027c0     4 OBJECT  LOCAL  HIDDEN     10 counter
# SYMBOLS:	 5: 0000000000000000   128 TLS     LOCAL  DEFAULT     8 counter_buffer

# DYNAMIC-LABEL: Dynamic section
# DYNAMIC:	 0x0000000000000017 (JMPREL)                    0x5830
# DYNAMIC-NEXT:  0x0000000000000002 (PLTRELSZ)                  48 (bytes)
# DYNAMIC-NEXT:  0x0000000000000003 (PLTGOT)                    0x8940
# DYNAMIC-NEXT:  0x0000000000000014 (PLTREL)                    RELA

#--- one.s

	.bss
	.global counter
	.type	counter, @object
	.hidden	counter
counter:
	.zero	4
	.size	counter, 4

	.text
	.global set_counter
	.type	set_counter, @function
set_counter:
	clgc	ca1, counter
	sw	a0, 0(ca1)
	ret
	.size	set_counter, . - set_counter

	.text
	.global increment_counter
	.type	increment_counter, @function
increment_counter:
	clgc	ca1, counter
	lw	a2, 0(ca1)
	addw	a2, a2, a0
	sw	a2, 0(ca1)
	ret
	.size	increment_counter, . - increment_counter

#--- oneb.s

	.text
	.global decrement_counter
	.type	decrement_counter, @function
decrement_counter:
	clgc	ca1, counter
	lw	a2, 0(ca1)
	subw	a2, a2, a0
	sw	a2, 0(ca1)
	ret
	.size	decrement_counter, . - decrement_counter

#--- two.s

	.section	.tbss,"awT",@nobits
	.type	counter_buffer, @object
counter_buffer:
	.zero	128
	.size	counter_buffer, 128

	.rodata
	.type	fmt,@object
fmt:
	.asciz	"%s:%d"
	.size	fmt, . - fmt

	.text
	.global counter_str
	.type	counter_str, @function
counter_str:
	cincoffset	csp, csp, -64
	sc	cra, 48(csp)
	sc	ca0, 0(csp)
1:	auipcc	ca0, %tls_gd_pcrel_hi(counter_buffer)
	cincoffset	ca0, ca0, %pcrel_lo(1b)
	ccall	__tls_get_addr
	sc	ca0, 32(csp)
	clgc	ca1, counter
	lw	a1, 0(ca1)
	sd	a1, 16(csp)
	clgc	ca2, fmt
	li	a1, 128
	ccall	snprintf
	lc	cra, 48(csp)
	lc	ca0, 32(csp)
	cincoffset	csp, csp, 64
	ret
	.size	counter_str, . - counter_str
