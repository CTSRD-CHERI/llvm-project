# RUN: llvm-mc -triple riscv32 -mattr=+c,+y,+cap-mode -show-encoding < %s \
# RUN:   | FileCheck -check-prefixes=CHECK,CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+y,+cap-mode -show-encoding \
# RUN:   -riscv-no-aliases < %s | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+y,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+y,+cap-mode,-xcheri -d - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+y,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+y,+cap-mode -d -M no-aliases - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-INST %s

# CHECK-BYTES: e0 1f
# CHECK-ALIAS: addiy cs0, csp, 1020
# CHECK-INST: c.cincoffset4cspn cs0, csp, 1020
# CHECK: # encoding:  [0xe0,0x1f]
addiy cs0, csp, 1020

# CHECK-BYTES: a0 7b
# CHECK-ALIAS: ly cs0, 112(ca5)
# CHECK-INST: c.lc
# CHECK64: # encoding: [0xa0,0x7b]
ly cs0, 112(ca5)

# CHECK-BYTES: a0 fb
# CHECK-ALIAS: sy cs0, 112(ca5)
# CHECK-INST: c.sc
# CHECK: # encoding: [0xa0,0xfb]
sy cs0, 112(ca5)

# CHECK-BYTES: 39 71
# CHECK-ALIAS: addiy csp, csp, -64
# CHECK-INST: c.cincoffset16csp csp, -64
# CHECK:  # encoding: [0x39,0x71]
addiy csp, csp, -64

# CHECK-BYTES: ce 70
# CHECK-ALIAS: ly cra, 240(csp)
# CHECK-INST: c.lcsp
# CHECK: # encoding: [0xce,0x70]
ly cra, 240(csp)

# CHECK-BYTES: 86 f9
# CHECK-ALIAS: sy cra, 240(csp)
# CHECK-INST: c.scsp
# CHECK: # encoding: [0x86,0xf9]
sc cra, 240(csp)
