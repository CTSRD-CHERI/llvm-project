# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/dice.s -o %t/dice.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/dice.o %t/one.o %t/two.o -o %t/libcrosscall.so.0

# RUN: llvm-readelf -t %t/libcrosscall.so.0 | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-readelf -s %t/libcrosscall.so.0 | FileCheck --check-prefix=SYMBOLS %s
# RUN: llvm-readelf --cap-relocs %t/libcrosscall.so.0 | FileCheck --check-prefix=CAPRELOCS %s
# RUN: llvm-objdump -d %t/libcrosscall.so.0 | FileCheck --check-prefix=DISASM %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:	  [ 8] .text.one
# SECTIONS:	  [ 9] .got.one
# SECTIONS:	  [11] .text.two
# SECTIONS:	  [12] .iplt.two
# SECTIONS-NEXT:      PROGBITS        0000000000004630
# SECTIONS:	  [13] .got.plt.two

# SYMBOLS-LABEL: Symbol table '.symtab'
# SYMBOLS:	    3: 0000000000002590     8 FUNC    LOCAL  HIDDEN      8 dice_roll
# SYMBOLS:	    5: 0000000000002598    44 FUNC    GLOBAL DEFAULT     8 one_value
# SYMBOLS-NEXT:     6: 00000000000025c4    12 FUNC    GLOBAL DEFAULT     8 one_func
# SYMBOLS-NEXT:     7: 00000000000045f0    44 FUNC    GLOBAL DEFAULT    11 two_value
# SYMBOLS-NEXT:     8: 000000000000461c    12 FUNC    GLOBAL DEFAULT    11 two_func

# CAPRELOCS-LABEL: CHERI __cap_relocs [
# CAPRELOCS-NEXT:     0x0035e0 Base: 0x2590 (dice_roll) Length: 4192 Perms: Function
# CAPRELOCS-NEXT:     0x005640 Base: 0x2590 (dice_roll) Length: 4192 Perms: Function

# one_value() should call dice_roll() directly
# 0x25a8 - 24 == 0x2590 (dice_roll)
# DISASM-LABEL: 0000000000002598 <one_value>:
# DISASM:	     25a8: 97 00 00 00   auipcc  cra, 0
# DISASM-NEXT:	     25ac: e7 80 80 fe   jalr    -24(cra)

# two_value() should branch to the IPLT entry
# 0x4600 + 48 == 0x4630 (.iplt.two)
# DISASM-LABEL: 00000000000045f0 <two_value>:
# DISASM:	     4600: 97 00 00 00   auipcc  cra, 0
# DISASM-NEXT:	     4604: e7 80 00 03   jalr    48(cra)

#--- dice.s

	.text
	.global	dice_roll
	.type	dice_roll, @function
	.hidden	dice_roll
dice_roll:
	li	a0, 4
	ret
	.size	dice_roll, . - dice_roll

#--- one.s

	.text
	.global	one_value
	.type	one_value, @function
one_value:
	cincoffset	csp, csp, -32
	sc	cra, 16(csp)
	sc	cs0, 0(csp)
	mv	s0, a0
	ccall	dice_roll
	addw	a0, a0, s0
	lc	cra, 16(csp)
	lc	cs0, 0(csp)
	cincoffset	csp, csp, 32
	ret
	.size	one_value, . - one_value

	.global	one_func
	.type	one_func, @function
one_func:
	clgc	ca0, dice_roll
	ret
	.size	one_func, . - one_func

#--- two.s

	.text
	.global	two_value
	.type	two_value, @function
two_value:
	cincoffset	csp, csp, -32
	sc	cra, 16(csp)
	sc	cs0, 0(csp)
	mv	s0, a0
	ccall	dice_roll
	or	a0, a0, s0
	lc	cra, 16(csp)
	lc	cs0, 0(csp)
	cincoffset	csp, csp, 32
	ret
	.size	two_value, . - two_value

	.global	two_func
	.type	two_func, @function
two_func:
	clgc	ca0, dice_roll
	ret
	.size	two_func, . - two_func

#--- compartments.json

{
    "compartments": {
	"one": { "files": ["one.o", "dice.o"] },
	"two": { "files": ["two.o"] }
    }
}
