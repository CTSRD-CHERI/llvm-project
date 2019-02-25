# RUN: not %cheri_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
# RUN: not %cheri_purecap_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
#
# Check that the assembler is able to handle capability memory instructions.
#

# Check that we don't crash when using a clc with an undefined string
clcbi $c3, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:12: error: expected immediate operand or valid relocation type
clc $c3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:15: error: expected immediate operand or valid relocation type
cld $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected immediate operand kind
clw $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected immediate operand kind
clhu $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:15: error: expected immediate operand kind
csb $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected immediate operand kind
cllc $c3, CR_EPCC
# CHECK: [[@LINE-1]]:11: error: expected general-purpose CHERI register operand or $ddc
csd $2, $3, CR_EPCC
# CHECK: [[@LINE-1]]:20: error: expecting capability register when parsing capability-relative address
clw $3, $29, 0(REG_FOO)
# CHECK: [[@LINE-1]]:16: error: expected general-purpose CHERI register operand or $ddc

# test that we don't allow the wrong reloations in CLC/CLCBI:
# corret width but not shifted:
clcbi $c3, %pcrel_lo(foo)($c11)
# CHECK: [[@LINE-1]]:12: error: expected both 20-bit signed immediate and multiple of 16
# need 20 bit shifted reloc
clcbi $c3, %capcall(foo)($c11)
# CHECK: [[@LINE-1]]:12: error: expected both 20-bit signed immediate and multiple of 16
# need 15 bit shifted value:
csc $c3, $29, %capcall20(foo)($c11)
# CHECK: [[@LINE-1]]:15: error: expected both 15-bit signed immediate and multiple of 16

# these are fine
csc $c3, $29, %captab(foo)($c11)
# CHECK: csc	$c3, $sp, %captab(foo)($c11) # encoding: [0xf8,0x6b,0b11101AAA,A]
# CHECK-NEXT: fixup A - offset: 0, value: %captab(foo), kind: fixup_CHERI_CAPTABLE11
clc $c3, $29, %capcall(foo)($c11)
# CHECK: clc	$c3, $sp, %capcall(foo)($c11) # encoding: [0xd8,0x6b,0b11101AAA,A]
# CHECK-NEXT: fixup A - offset: 0, value: %capcall(foo), kind: fixup_CHERI_CAPCALL11
clcbi $c3, %captab20(foo)($c11)
# CHECK: clcbi	$c3, %captab20(foo)($c11) # encoding: [0x74,0x6b,A,A]
# CHECK-NEXT: fixup A - offset: 0, value: %captab20(foo), kind: fixup_CHERI_CAPTABLE20
clcbi $c3, %capcall20(foo)($c11)
# CHECK: clcbi	$c3, %capcall20(foo)($c11) # encoding: [0x74,0x6b,A,A]
# CHECK-NEXT: fixup A - offset: 0, value: %capcall20(foo), kind: fixup_CHERI_CAPCALL20
