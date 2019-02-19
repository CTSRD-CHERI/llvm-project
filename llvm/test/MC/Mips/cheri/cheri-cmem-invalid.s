# RUN: not %cheri_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
# RUN: not %cheri_purecap_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
#
# Check that the assembler is able to handle capability memory instructions.
#

# Check that we don't crash when using a clc with an undefined string
clcbi $c3, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:12: error: expected both 20-bit signed immediate and multiple of 16
clc $c3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:15: error: expected both 15-bit signed immediate and multiple of 16
cld $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected both 11-bit signed immediate and multiple of 8
clw $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected both 10-bit signed immediate and multiple of 4
clhu $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:15: error: expected both 9-bit signed immediate and multiple of 2
csb $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:14: error: expected 8-bit signed immediate
cllc $c3, CR_EPCC
# CHECK: [[@LINE-1]]:11: error: expected general-purpose CHERI register operand or $ddc
csd $2, $3, CR_EPCC
# CHECK: [[@LINE-1]]:20: error: expecting capability register when parsing capability-relative address
clw $3, $29, 0(REG_FOO)
# CHECK: [[@LINE-1]]:16: error: expected general-purpose CHERI register operand or $ddc
