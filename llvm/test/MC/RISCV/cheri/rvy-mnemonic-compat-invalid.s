# RUN: not llvm-mc < %s -triple=riscv32 -mattr=+c,+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-INT %s --implicit-check-not="error:"
# RUN: not llvm-mc < %s -triple=riscv64 -mattr=+c,+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-INT %s --implicit-check-not="error:"
# RUN: not llvm-mc < %s --defsym CAPMODE=1 -triple=riscv32 -mattr=+c,+a,+xcheri,+cap-mode -riscv-no-aliases 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CAP %s --implicit-check-not="error:"
# RUN: not llvm-mc < %s --defsym CAPMODE=1 -triple=riscv64 -mattr=+c,+a,+xcheri,+cap-mode -riscv-no-aliases 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CAP %s --implicit-check-not="error:"

## Check that RVY mnemonics reject DDC as a YBLD/YSS operand for compatibility with the RVY specification.
# CHECK: :[[@LINE+1]]:10: error: operand must be a GPR other than x0
ybld x1, ddc, c3
# CHECK: :[[@LINE+1]]:14: error: operand must be a GPR
ybld x1, x2, ddc
# CHECK: :[[@LINE+1]]:9: error: operand must be a GPR other than x0
yss x1, ddc, x3
# CHECK: :[[@LINE+1]]:13: error: operand must be a GPR
yss x1, x2, ddc

## Check that in integer mode jumps reject capability registers.
# CHECK-INT: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jr c1
# CHECK-INT: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jalr c1, c2
# CHECK-INT: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jal c1, 20

## Normal loads should reject C base registers.
# CHECK-INT: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
lw x1, 0(c2)

## Check that XCheri mnemonics do not coerce arguments.
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
cgetbase x1, x2
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
cgetperm x1, x2
# CHECK: :[[@LINE+1]]:12: error: invalid operand for instruction
csetbounds x1, x2, x3
# CHECK: :[[@LINE+1]]:10: error: invalid operand for instruction
csetaddr x1, x2, x3
# CHECK: :[[@LINE+1]]:9: error: invalid operand for instruction
cinvoke x1, x2


## Check that we don't coerce arguments for .ddc and .cap instructions in integer and capability modes
# CHECK: :[[@LINE+1]]:8: error: invalid operand for instruction
lc.ddc x1, 0(x2)
# CHECK: :[[@LINE+1]]:8: error: invalid operand for instruction
sc.ddc x1, 0(x2)
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
lc.cap c1, 0(x2)
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
sc.cap c1, 0(x2)

## Check that RVY mnemonics reject explicit c registers in both integer and capability modes.
# CHECK: :[[@LINE+1]]:6: error: operand must be a GPR
yadd c1, x2, x3
# CHECK: :[[@LINE+1]]:10: error: operand must be a GPR
yadd x1, c2, x3
# CHECK: :[[@LINE+1]]:14: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo modifier or an integer in the range [-2048, 2047]
yadd x1, x2, c3

# CHECK: :[[@LINE+1]]:12: error: operand must be a GPR
ypermr x1, c2

# CHECK: :[[@LINE+1]]:6: error: operand must be a GPR
yhiw c1, x2, x3
# CHECK: :[[@LINE+1]]:10: error: operand must be a GPR
yhiw x1, c2, x3

# CHECK: :[[@LINE+1]]:6: error: operand must be a GPR
ybld c1, x2, x3
# CHECK: :[[@LINE+1]]:10: error: operand must be a GPR other than x0
ybld x1, c2, x3
# CHECK: :[[@LINE+1]]:14: error: operand must be a GPR
ybld x1, x2, c3

# CHECK: :[[@LINE+1]]:4: error: operand must be a GPR
ly c1, 0(x2)
# CHECK-INT: :[[@LINE+2]]:10: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:10: error: operand must be a GPR
ly x1, 0(c2)

# CHECK: :[[@LINE+1]]:4: error: operand must be a GPR
sy c1, 0(x2)
# CHECK-INT: :[[@LINE+2]]:10: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:10: error: operand must be a GPR
sy x1, 0(c2)

## Check that c.lysp and c.sysp reject csp instead of sp.
# CHECK-INT: :[[@LINE+2]]:15: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:15: error: operand must be sp
c.lysp a0, 16(csp)
# CHECK-INT: :[[@LINE+2]]:15: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:15: error: operand must be sp
c.sysp a0, 16(csp)
## Check that c.ly rejects csp (instead of sp).
# CHECK-INT: :[[@LINE+2]]:13: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:13: error: operand must be a GPR
c.ly a0, 16(ca0)
# CHECK-INT: :[[@LINE+2]]:13: error: invalid operand for instruction
# CHECK-CAP: :[[@LINE+1]]:13: error: operand must be a GPR
c.sy a0, 16(ca0)

