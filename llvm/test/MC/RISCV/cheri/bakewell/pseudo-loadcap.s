# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -d - | FileCheck --check-prefix=INSTR %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=RELOC %s
# RUN: llvm-mc -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=FIXUP %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -d - | FileCheck --check-prefix=INSTR %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=RELOC %s
# RUN: llvm-mc -triple riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=FIXUP %s

.option capmode

llc ca0, foo
# RELOC: R_RISCV_PCREL_HI20 foo 0x0
# RELOC: R_RISCV_PCREL_LO12_I <null> 0x0
# INSTR: auipc ca0, 0
# INSTR: caddi ca0, ca0, 0
# FIXUP: fixup A - offset: 0, value: %pcrel_hi(foo), kind: fixup_riscv_pcrel_hi20
# FIXUP: fixup A - offset: 0, value: %pcrel_lo(.Lpcrel_hi0), kind: fixup_riscv_pcrel_lo12_i

lgc ca0, bar
# RELOC: R_RISCV_CHERI_CAPTAB_PCREL_HI20 bar 0x0
# RELOC: R_RISCV_PCREL_LO12_I <null> 0x0
# INSTR: auipc ca0, 0
# INSTR: lc ca0, 0(ca0)
# FIXUP: fixup A - offset: 0, value: %captab_pcrel_hi(bar), kind: fixup_riscv_captab_pcrel_hi20
# FIXUP: fixup A - offset: 0, value: %pcrel_lo(.Lpcrel_hi1), kind: fixup_riscv_pcrel_lo12_i

