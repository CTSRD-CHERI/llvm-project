# RUN: llvm-mc %s -triple=riscv32 -mattr=+c,+xcheri,+cap-mode,+f,+d  -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+c,+xcheri,+cap-mode,+f,+d < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode,+f,+d  -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

## Floating-point store is only supported in capmode for RV32

# CHECK-ASM-AND-OBJ: c.cfsd fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x08,0xb5]
c.cfsd fa0, 40(ca0)
# CHECK-ASM-AND-OBJ: c.cfld fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x08,0x35]
c.cfld fa0, 40(ca0)

# CHECK-ASM-AND-OBJ: c.cfsdcsp ft1, 40(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xb4]
c.cfsdcsp ft1, 40(csp)
# CHECK-ASM-AND-OBJ: c.cfldcsp ft1, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x30]
c.cfldcsp ft1, 40(csp)
