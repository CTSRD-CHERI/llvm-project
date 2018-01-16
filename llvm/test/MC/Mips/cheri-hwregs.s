# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: not %cheri_llvm-mc %s -show-encoding -defsym=BAD=1 2>&1 | FileCheck %s -check-prefix=ERR

# 0x37f = ﻿hex(0x3f + (0xd << 6))
CReadHwr $c1, $0
# CHECK: creadhwr	$c1, $0
# CHECK-SAME: encoding: [0x48,0x01,0x03,0x7f]
CReadHwr $c1, $31
# CHECK: creadhwr	$c1, $31
# CHECK-SAME: encoding: [0x48,0x01,0xfb,0x7f]

# 0x3bf = ﻿hex(0x3f + (0xe << 6))
CWriteHwr $c1, $0
# CHECK: cwritehwr	$c1, $0
# CHECK-SAME: encoding: [0x48,0x01,0x03,0xbf]
CWriteHwr $c1, $31
# CHECK: cwritehwr	$c1, $31
# CHECK-SAME: encoding: [0x48,0x01,0xfb,0xbf]

.ifdef BAD
CWriteHwr $c1, $hwr_cpunum  # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, $c2          # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, $32          # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, 0            # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, -1           # ERR: [[@LINE]]:16: error: invalid operand for instruction
.endif
