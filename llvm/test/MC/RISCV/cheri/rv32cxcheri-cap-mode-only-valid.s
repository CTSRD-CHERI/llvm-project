# RUN: llvm-mc %s -triple=riscv32 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+c,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

## Different encoding between RV64 and RV32 (see rv64cxcheri-cap-mode-only-valid.s)

# CHECK-ASM-AND-OBJ: c.clccsp cra, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x70]
c.clccsp cra, 40(csp)
# CHECK-ASM-AND-OBJ: c.csccsp cra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xe2]
c.csccsp cra, 256(csp)
# CHECK-ASM-AND-OBJ: c.clc ca2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x69]
c.clc ca2, 16(ca0)
# CHECK-ASM-AND-OBJ: c.csc ca5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xe2]
c.csc ca5, 128(ca3)

## C.JAL is only defined for RV32C:
# CHECK-OBJ: c.cjal 0x806
# CHECK-ASM: c.cjal 2046
# CHECK-ASM-SAME: encoding: [0xfd,0x2f]
c.cjal 2046
