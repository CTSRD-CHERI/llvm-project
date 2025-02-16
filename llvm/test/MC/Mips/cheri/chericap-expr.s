# RUN: llvm-mc -triple mips64-unknown-freebsd -mattr=+cheri128 %s -o - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple mips64-unknown-freebsd -mattr=+cheri128 %s -o - \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=CHECK-RELOC %s

foo:
  nop
  nop
bar:
  jr $ra

.data

# CHECK-INST: .chericap foo-foo
# CHECK-RELOC-NOT: R_MIPS_CHERI_CAPABILITY
.chericap foo - foo

# CHECK-INST: .chericap foo-bar
# CHECK-RELOC-NOT: R_MIPS_CHERI_CAPABILITY
.chericap foo - bar

# CHECK-INST: .chericap foo
# CHECK-RELOC: 0x20 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
.chericap foo

# CHECK-INST: .chericap foo+4
# CHECK-RELOC: 0x30 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x4
.chericap foo + 4

# CHECK-INST: .chericap foo+(bar-foo)
# CHECK-RELOC: 0x40 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x8
.chericap foo + (bar - foo)
