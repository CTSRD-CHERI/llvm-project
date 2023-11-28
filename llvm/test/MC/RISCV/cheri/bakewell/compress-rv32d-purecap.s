# RUN: llvm-mc -triple riscv32 -mattr=+c,+d,+zcheripurecap,+cap-mode -show-encoding < %s \
# RUN:   | FileCheck -check-prefixes=CHECK,CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+d,+zcheripurecap,+cap-mode -show-encoding \
# RUN:   -riscv-no-aliases < %s | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+d,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+d,+zcheripurecap,+cap-mode -d - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+d,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+d,+zcheripurecap,+cap-mode -d -M no-aliases - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-INST %s

# Tests instructions available in purecap + C + D, rv32 only.

# CHECK-BYTES: 06 20
# CHECK-ALIAS: fld ft0, 64(csp)
# CHECK-INST: c.cfldcsp ft0, 64(csp)
# CHECK: # encoding:  [0x06,0x20]
fld ft0, 64(csp)

# CHECK-BYTES: 82 a0
# CHECK-ALIAS: fsd ft0, 64(csp)
# CHECK-INST: c.cfsdcsp ft0, 64(csp)
# CHECK: # encoding:  [0x82,0xa0]
fsd ft0, 64(csp)

# CHECK-BYTES: 60 3c
# CHECK-ALIAS: fld fs0, 248(cs0)
# CHECK-INST: c.cfld fs0, 248(cs0)
# CHECK: # encoding:  [0x60,0x3c]
fld fs0, 248(cs0)

# CHECK-BYTES: 60 bc
# CHECK-ALIAS: fsd fs0, 248(cs0)
# CHECK-INST: c.cfsd fs0, 248(cs0)
# CHECK: # encoding:  [0x60,0xbc]
fsd fs0, 248(cs0)
