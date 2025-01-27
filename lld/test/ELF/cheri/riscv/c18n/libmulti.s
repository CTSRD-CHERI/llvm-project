# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/oneb.s -o %t/oneb.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o %t/oneb.o %t/two.o -o %t/libmulti.so.0

# RUN: llvm-readelf -t %t/libmulti.so.0 | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-readelf -l %t/libmulti.so.0 | FileCheck --check-prefix=SEGMENTS %s
# RUN: llvm-readelf -s %t/libmulti.so.0 | FileCheck --check-prefix=SYMBOLS %s
# RUN: llvm-readelf -d %t/libmulti.so.0 | FileCheck --check-prefix=DYNAMIC %s

# Legacy version linked via --compartment
# RUN: ld.lld --shared --compartment=one %t/one.o %t/oneb.o --compartment=two %t/two.o -o %t/libmulti_legacy.so.0

# RUN: llvm-readelf -t %t/libmulti_legacy.so.0 | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-readelf -l %t/libmulti_legacy.so.0 | FileCheck --check-prefix=SEGMENTS %s
# RUN: llvm-readelf -s %t/libmulti_legacy.so.0 | FileCheck --check-prefix=SYMBOLS %s
# RUN: llvm-readelf -d %t/libmulti_legacy.so.0 | FileCheck --check-prefix=DYNAMIC %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:	[ 6] .c18nstrtab
# SECTIONS:	[12] .text.one
# SECTIONS:	[13] .got.one
# SECTIONS:	[14] .rela.plt.two
# SECTIONS:	[15] .text.two
# SECTIONS:	[16] .plt.two
# SECTIONS:	[17] .got.two
# SECTIONS:	[18] .got.plt.two

# SEGMENTS:	There are 21 program headers, starting at offset 64

# SEGMENTS-LABEL: Program Headers:
# SEGMENTS-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# SEGMENTS-NEXT:    PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x000498 0x000498 R   0x8
# SEGMENTS-NEXT:    LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x000738 0x000738 R   0x1000
# SEGMENTS-NEXT:    LOAD           0x000738 0x0000000000001738 0x0000000000001738 0x000120 0x000120 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x000858 0x0000000000002858 0x0000000000002858 0x000000 0x000004 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x000860 0x0000000000003860 0x0000000000003860 0x000040 0x000040 R E 0x1000
# SEGMENTS-NEXT:    LOAD           0x0008a0 0x00000000000048a0 0x00000000000048a0 0x000020 0x000020 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x0008c0 0x00000000000058c0 0x00000000000058c0 0x000030 0x000030 R   0x1000
# SEGMENTS-NEXT:    LOAD           0x0008f0 0x00000000000068f0 0x00000000000068f0 0x0000a0 0x0000a0 R E 0x1000
# SEGMENTS-NEXT:    LOAD           0x000990 0x0000000000007990 0x0000000000007990 0x000040 0x000040 RW  0x1000
# SEGMENTS-NEXT:    LOAD           0x0009d0 0x00000000000089d0 0x00000000000089d0 0x000040 0x000040 RW  0x1000
# SEGMENTS-NEXT:    TLS            0x000738 0x0000000000000738 0x0000000000000738 0x000000 0x000080 R   0x1
# SEGMENTS-NEXT:    DYNAMIC        0x000738 0x0000000000001738 0x0000000000001738 0x000120 0x000120 RW  0x8
# SEGMENTS-NEXT:    C18N_NAME      0x000860 0x0000000000003860 0x0000000000000001 0x000060 0x001060     0x10
# SEGMENTS-NEXT:    C18N_NAME      0x0008c0 0x00000000000058c0 0x0000000000000005 0x000150 0x003150     0x10
# SEGMENTS-NEXT:    GNU_RELRO      0x000738 0x0000000000001738 0x0000000000001738 0x000120 0x0008c8 R   0x1
# SEGMENTS-NEXT:    GNU_RELRO      0x0008a0 0x00000000000048a0 0x00000000000048a0 0x000020 0x000760 R   0x1
# SEGMENTS-NEXT:    GNU_RELRO      0x000990 0x0000000000007990 0x0000000000007990 0x000040 0x000670 R   0x1
# SEGMENTS-NEXT:    CHERI_PCC      0x000669 0x0000000000000669 0x0000000000000669 0x000006 0x000006 R E 0x1
# SEGMENTS-NEXT:    CHERI_PCC      0x000860 0x0000000000003860 0x0000000000003860 0x000060 0x001060 R E 0x10
# SEGMENTS-NEXT:    CHERI_PCC      0x0008f0 0x00000000000068f0 0x00000000000068f0 0x000120 0x002120 R E 0x10
# SEGMENTS-NEXT:    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# SEGMENTS-LABEL: Section to Segment mapping:
# SEGMENTS-NEXT:  Segment Sections...
# SEGMENTS-NEXT:   00
# SEGMENTS-NEXT:   01     .dynsym .gnu.hash .hash .dynstr .rela.dyn .c18nstrtab  .rodata __cap_relocs
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
# SEGMENTS-NEXT:   12     .text.one .got.one
# SEGMENTS-NEXT:   13     .rela.plt.two .text.two .plt.two .got.two .got.plt.two
# SEGMENTS-NEXT:   14     .dynamic
# SEGMENTS-NEXT:   15     .got.one
# SEGMENTS-NEXT:   16     .got.two
# SEGMENTS-NEXT:   17     .rodata
# SEGMENTS-NEXT:   18     .text.one .got.one
# SEGMENTS-NEXT:   19     .text.two .plt.two .got.two .got.plt.two
# SEGMENTS-NEXT:   20
# SEGMENTS-NEXT:   None   .comment .symtab .shstrtab .strtab

# SYMBOLS-LABEL: Symbol table '.dynsym'
# SYMBOLS:	 3: 0000000000003860    16 FUNC    GLOBAL DEFAULT    12 set_counter
# SYMBOLS-NEXT:  4: 0000000000003870    24 FUNC    GLOBAL DEFAULT    12 increment_counter
# SYMBOLS-NEXT:  5: 0000000000003888    24 FUNC    GLOBAL DEFAULT    12 decrement_counter
# SYMBOLS-NEXT:  6: 00000000000068f0    84 FUNC    GLOBAL DEFAULT    15 counter_str

# SYMBOLS-LABEL: Symbol table '.symtab'
# SYMBOLS:	 3: 0000000000002858     4 OBJECT  LOCAL  HIDDEN     11 counter
# SYMBOLS:	 5: 0000000000000000   128 TLS     LOCAL  DEFAULT     9 counter_buffer

# DYNAMIC-LABEL: Dynamic section
# DYNAMIC:	 0x0000000000000017 (JMPREL)                    0x58c0
# DYNAMIC-NEXT:  0x0000000000000002 (PLTRELSZ)                  48 (bytes)
# DYNAMIC-NEXT:  0x0000000000000003 (PLTGOT)                    0x89d0
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

#--- compartments.json

{
    "compartments": {
	"one": { "files": ["*/one.*o", "*/oneb.*o"] },
	"two": { "files": [], "symbols": ["counter_str"] }
    }
}
