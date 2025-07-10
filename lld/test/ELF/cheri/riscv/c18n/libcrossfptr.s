# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/dice.s -o %t/dice.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/dice.o %t/one.o %t/two.o -o %t/libcrossfptr.so.0

# RUN: llvm-readelf -t %t/libcrossfptr.so.0 | FileCheck --check-prefix=SECTIONS %s
# RUN: llvm-readelf -s %t/libcrossfptr.so.0 | FileCheck --check-prefix=SYMBOLS %s
# RUN: llvm-readelf --cap-relocs %t/libcrossfptr.so.0 | FileCheck --check-prefix=CAPRELOCS %s

# SECTIONS-LABEL: Section Headers:
# SECTIONS:	  [ 8] .text.one
# SECTIONS:	  [ 9] .got.one
# SECTIONS-NEXT:       PROGBITS        0000000000003580
# SECTIONS:	  [10] .text.two
# SECTIONS:	  [11] .got.two
# SECTIONS-NEXT:       PROGBITS        00000000000055b0

# SYMBOLS-LABEL: Symbol table '.symtab'
# SYMBOLS:	    3: 0000000000002568     8 FUNC    LOCAL  HIDDEN      8 dice_roll
# SYMBOLS:          5: 0000000000002570    12 FUNC    GLOBAL DEFAULT     8 one_func
# SYMBOLS-NEXT:     6: 00000000000045a0    12 FUNC    GLOBAL DEFAULT    10 two_func

# CAPRELOCS-LABEL: CHERI __cap_relocs [
# CAPRELOCS-NEXT:     0x003590 Base: 0x2568 (dice_roll+0) Length: 8 Perms: Function
# CAPRELOCS-NEXT:     0x0055c0 Base: 0x2568 (dice_roll+0) Length: 8 Perms: Function

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
	.global	one_func
	.type	one_func, @function
one_func:
	clgc	ca0, dice_roll
	ret
	.size	one_func, . - one_func

#--- two.s

	.text
	.global	two_func
	.type	two_func, @function
two_func:
	clgc	ca0, dice_roll
	ret
	.size	two_func, . - two_func

#--- compartments.json

{
    "compartments": {
	"one": { "files": ["*/one.o", "*/dice.o"] },
	"two": { "files": ["*/two.o"] }
    }
}
