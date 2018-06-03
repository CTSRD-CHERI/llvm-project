# Check that it assembles if I no longer defined INCLUDE_BAD
# RUN: not %cheri_llvm-mc %s -defsym=INCLUDE_BAD=1 -show-encoding -cheri-strict-c0-asm -filetype=null -o /dev/null 2>&1 | FileCheck %s
# Check that we assemble successfully if we exclude the bad instrs
# RUN: %cheri_llvm-mc %s -show-encoding -cheri-strict-c0-asm -filetype=obj -o /dev/null 2>&1

.ifdef INCLUDE_BAD
cmove $c0, $c1  # CHECK: [[@LINE]]:7: error: register name $c0 is invalid as this operand.
cmove $ddc, $c1  # CHECK: [[@LINE]]:7: error: register name $ddc is invalid as this operand. Did you mean $cnull?
cmove $c2, $c0  # CHECK: [[@LINE]]:12: error: register name $c0 is invalid as this operand.
cmove $c2, $ddc  # CHECK: [[@LINE]]:12: error: register name $ddc is invalid as this operand. Did you mean $cnull?
cincoffset $c0, $c0, 2  # CHECK: [[@LINE]]:12: error: register name $c0 is invalid as this operand.

cgetdefault $c0 # CHECK: [[@LINE]]:13: error: register name $c0 is invalid as this operand.
cgetepcc $c0    # CHECK: [[@LINE]]:10: error: register name $c0 is invalid as this operand.



# Using $c0 as a non-source-address register should be an error:
cfromptr $c0, $c1, $3  # CHECK: 39:10: error: register name $c0 is invalid as this operand.
ctoptr $3, $c0, $c2    # CHECK: [[@LINE]]:12: error: register name $c0 is invalid as this operand.
clc $c0, $zero, 0($c2) # CHECK: [[@LINE]]:5: error: register name $c0 is invalid as this operand.
csc $c0, $zero, 0($c2) # CHECK: [[@LINE]]:5: error: register name $c0 is invalid as this operand.
cllc $c0, $c1          # CHECK: [[@LINE]]:6: error: register name $c0 is invalid as this operand.
cscc $2, $c0, $c1      # CHECK: [[@LINE]]:10: error: register name $c0 is invalid as this operand.
.endif


# However, cfromptr, ctoptr, cl* and cs* should still work with $c0 as the address register
cfromptr $c1, $c0, $3 # CHECK: [[@LINE]]:15: warning: register name $c0 is deprecated. Use $ddc instead.
ctoptr $3, $c1, $c0    # CHECK: [[@LINE]]:17: warning: register name $c0 is deprecated. Use $ddc instead.
clc $c1, $zero, 0($c0)  # CHECK: [[@LINE]]:19: warning: register name $c0 is deprecated. Use $ddc instead.
clcbi $c1, 0($c0)  # CHECK: [[@LINE]]:14: warning: register name $c0 is deprecated. Use $ddc instead.
clw $3, $zero, 0($c0)  # CHECK: [[@LINE]]:18: warning: register name $c0 is deprecated. Use $ddc instead.
csc $c1, $zero, 0($c0)  # CHECK: [[@LINE]]:19: warning: register name $c0 is deprecated. Use $ddc instead.
cscbi $c1, 0($c0)  # CHECK: [[@LINE]]:14: warning: register name $c0 is deprecated. Use $ddc instead.
csd $3, $zero, 0($c0)  # CHECK: [[@LINE]]:18: warning: register name $c0 is deprecated. Use $ddc instead.
cllc $c1, $c0  # CHECK: [[@LINE]]:11: warning: register name $c0 is deprecated. Use $ddc instead.
cllbu $2, $c0  # CHECK: [[@LINE]]:11: warning: register name $c0 is deprecated. Use $ddc instead.
cscc $2, $c1, $c0  # CHECK: [[@LINE]]:15: warning: register name $c0 is deprecated. Use $ddc instead.
cscb $2, $3, $c0  # CHECK: [[@LINE]]:14: warning: register name $c0 is deprecated. Use $ddc instead.
