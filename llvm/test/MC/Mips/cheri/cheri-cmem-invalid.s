# RUN: not %cheri_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
# RUN: not %cheri_purecap_llvm-mc %s -show-encoding 2>&1 | FileCheck %s
#
# Check that the assembler is able to handle capability memory instructions.
#

# Check that we don't crash when using a clc with an undefined string
clcbi $c3, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
clc $c3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
cld $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
clw $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
clhu $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
csb $3, $29, CR_EPCC($c11)
# CHECK: [[@LINE-1]]:1: error: Cannot expand symbol in immediate operand for this instruction
cllc $c3, CR_EPCC
# CHECK: [[@LINE-1]]:11: error: expected general-purpose CHERI register operand or $ddc
csd $2, $3, CR_EPCC
# CHECK: [[@LINE-1]]:20: error: expecting capability register when parsing capability-relative address
clw $3, $29, 0(REG_FOO)
# CHECK: [[@LINE-1]]:16: error: expected general-purpose CHERI register operand or $ddc
