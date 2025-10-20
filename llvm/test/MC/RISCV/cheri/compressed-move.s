## Check that we don't compress addi 0 to c.mv (which is c.cmove for the RISC-V standard capmode)
# RUN: llvm-mc %s -triple=riscv64 -mattr=+c,+xcheri,-cap-mode -riscv-no-aliases \
# RUN:    -show-encoding --defsym=PURECAP=0 | FileCheck %s --check-prefix=INT-MODE
# RUN: llvm-mc %s -triple=riscv64 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases \
# RUN:    -show-encoding --defsym=PURECAP=1 | FileCheck %s --check-prefix=CAP-MODE

# TODO-ISAV9-CAPMODE: c.mv a1, a0    # encoding: [0xaa,0x85]
# TODO-ISAV9-INTMODE: c.mv a1, a0    # encoding: [0xaa,0x85]
# CAP-MODE:  addi a1, a0, 0 # encoding: [0x93,0x05,0x05,0x00]
# INT-MODE:  c.mv a1, a0    # encoding: [0xaa,0x85]
## Should not be compressed to c.cmove for capability mode (only in integer mode).
addi a1, a0, 0

# CMove should be compressed for capmode.
# TODO-ISAV9-CAPMODE: cmove ca1, ca0   # encoding: [0xdb,0x05,0xa5,0xfe]
# TODO-ISAV9-INTMODE: cmove ca1, ca0   # encoding: [0xdb,0x05,0xa5,0xfe]
# INT-MODE:  cmove ca1, ca0   # encoding: [0xdb,0x05,0xa5,0xfe]
# CAP-MODE:  c.cmove ca1, ca0 # encoding: [0xaa,0x85]
cmove ca1, ca0

.if PURECAP
# CAP-MODE:  c.cmove ca2, ca0 # encoding: [0x2a,0x86]
c.cmove ca2, ca0
.endif
