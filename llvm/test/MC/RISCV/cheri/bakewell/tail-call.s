# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump -d - | FileCheck --check-prefix=INSTR %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=RELOC %s
# RUN: llvm-mc -triple riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=FIXUP %s

.option capmode

.long foo

tail foo
# RELOC: R_RISCV_CHERI_CCALL foo 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: foo, kind: fixup_riscv_ccall
tail bar
# RELOC: R_RISCV_CHERI_CCALL bar 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: bar, kind: fixup_riscv_ccall

# Ensure that tailcalls to functions whose names coincide with register names work.

tail zero
# RELOC: R_RISCV_CHERI_CCALL zero 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: zero, kind: fixup_riscv_ccall

tail f1
# RELOC: R_RISCV_CHERI_CCALL f1 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: f1, kind: fixup_riscv_ccall

tail ra
# RELOC: R_RISCV_CHERI_CCALL ra 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: ra, kind: fixup_riscv_ccall

tail mstatus
# RELOC: R_RISCV_CHERI_CCALL mstatus 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: mstatus, kind: fixup_riscv_ccall

# Ensure that tailcalls to procedure linkage table symbols work.

tail foo@plt
# RELOC: R_RISCV_CHERI_CCALL foo 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: foo, kind: fixup_riscv_ccall

.option nocapmode

tail foo
# RELOC: R_RISCV_CALL_PLT foo 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: foo, kind: fixup_riscv_call
tail bar
# RELOC: R_RISCV_CALL_PLT bar 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: bar, kind: fixup_riscv_call

# Ensure that calls to functions whose names coincide with register names work.

tail zero
# RELOC: R_RISCV_CALL_PLT zero 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: zero, kind: fixup_riscv_call

tail f1
# RELOC: R_RISCV_CALL_PLT f1 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: f1, kind: fixup_riscv_call

tail ra
# RELOC: R_RISCV_CALL_PLT ra 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: ra, kind: fixup_riscv_call

tail mstatus
# RELOC: R_RISCV_CALL_PLT mstatus 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: mstatus, kind: fixup_riscv_call

# Ensure that calls to procedure linkage table symbols work.

tail foo@plt
# RELOC: R_RISCV_CALL_PLT foo 0x0
# INSTR: auipc t1, 0
# INSTR: jr t1
# FIXUP: fixup A - offset: 0, value: foo@plt, kind: fixup_riscv_call_plt
