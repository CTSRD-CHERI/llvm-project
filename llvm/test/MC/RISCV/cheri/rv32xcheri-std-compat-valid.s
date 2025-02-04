# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri-std-compat -riscv-no-aliases -show-encoding
# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri-std-compat -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+xcheri-std-compat -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s

# RUN: not llvm-mc %s --triple=riscv64 --mattr=+xcheri,-xcheri-std-compat --filetype=obj -o /dev/null 2>&1 \
# RUN:     | FileCheck --check-prefix=NO-STD-COMPAT-ERROR --implicit-check-not="error:" %s

# CHECK-INST: modesw.cap
# NO-STD-COMPAT-ERROR: [[#@LINE+4]]:1: error: instruction requires the following: CHERI standard-compatible subset{{$}}
# NO-STD-COMPAT-ERROR-NEXT: modesw.cap
# NO-STD-COMPAT-ERROR-NEXT: ^{{$}}
# CHECK-SAME: encoding: [0x33,0x10,0x00,0x12]
modesw.cap
# CHECK-INST: modesw.int
# NO-STD-COMPAT-ERROR: [[#@LINE+4]]:1: error: instruction requires the following: CHERI standard-compatible subset{{$}}
# NO-STD-COMPAT-ERROR-NEXT: modesw.int
# NO-STD-COMPAT-ERROR-NEXT: ^{{$}}
# CHECK-SAME: encoding: [0x33,0x10,0x00,0x14]
modesw.int

# Should not see any further errors
# NO-STD-COMPAT-ERROR-EMPTY:
