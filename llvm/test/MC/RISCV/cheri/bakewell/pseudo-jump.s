# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump -d - | FileCheck --check-prefix=INSTR %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=RELOC %s
# RUN: llvm-mc -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=FIXUP %s

.long foo

.option capmode

jump foo, ct6
# RELOC: R_RISCV_CHERI_CCALL foo 0x0
# INSTR: auipc t6, 0
# INSTR: jr  t6
# FIXUP: fixup A - offset: 0, value: foo, kind: fixup_riscv_ccall

# Ensure that jumps to symbols whose names coincide with register names work.

jump zero, cra
# RELOC: R_RISCV_CHERI_CCALL zero 0x0
# INSTR: auipc ra, 0
# INSTR: ret
# FIXUP: fixup A - offset: 0, value: zero, kind: fixup_riscv_ccall

1:
jump 1b, ct6
# INSTR: auipc t6, 0
# INSTR: jr  t6
# FIXUP: fixup A - offset: 0, value: .Ltmp0, kind: fixup_riscv_ccall

.option nocapmode

jump foo, x31
# RELOC: R_RISCV_CALL_PLT foo 0x0
# INSTR: auipc t6, 0
# INSTR: jr  t6
# FIXUP: fixup A - offset: 0, value: foo, kind: fixup_riscv_call

# Ensure that jumps to symbols whose names coincide with register names work.

jump zero, x1
# RELOC: R_RISCV_CALL_PLT zero 0x0
# INSTR: auipc ra, 0
# INSTR: ret
# FIXUP: fixup A - offset: 0, value: zero, kind: fixup_riscv_call

2:
jump 2b, x31
# INSTR: auipc t6, 0
# INSTR: jr  t6
# FIXUP: fixup A - offset: 0, value: .Ltmp1, kind: fixup_riscv_call


