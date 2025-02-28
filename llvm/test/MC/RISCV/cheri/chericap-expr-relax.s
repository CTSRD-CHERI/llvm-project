# RUN: llvm-mc -triple=riscv64 -mattr=+c,+xcheri < %s \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+c,+xcheri < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=CHECK-RELOC %s

foo:
  nop
  j bar
bar:
  ret

.data

# CHECK-INST: .chericap foo+(bar-foo)
# CHECK-RELOC: 0x0 R_RISCV_CHERI_CAPABILITY foo 0x4
.chericap foo + (bar - foo)
