## Test .attribute effects.
## We do not provide '-mattr=' and '.option rvc' and enable extensions through
## '.attribute arch'.

# RUN: llvm-mc -triple riscv32 -filetype=obj %s \
# RUN:   | llvm-objdump --triple=riscv32 -d -M no-aliases - \
# RUN:   | FileCheck -check-prefix=CHECK-INST %s

# RUN: llvm-mc -triple riscv64 -filetype=obj %s \
# RUN:   | llvm-objdump --triple=riscv64 -d -M no-aliases - \
# RUN:   | FileCheck -check-prefix=CHECK-INST %s

.attribute arch, "rv64i2p1_xcheri0p0"

# CHECK-INST: cincoffset ct0, ct1, 0
cincoffset ct0, ct1, 0
