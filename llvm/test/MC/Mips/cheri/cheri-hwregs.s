# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: not %cheri_llvm-mc %s -show-encoding -defsym=BAD=1 2>&1 | FileCheck %s -check-prefix=ERR
# Check that we don't crash when disassembling:
# RUN: %cheri_llvm-mc %s -filetype=obj -o - | llvm-objdump -d - > /dev/null

# 0x37f = ﻿hex(0x3f + (0xd << 6))
CReadHwr $c1, $0
# CHECK: creadhwr	$c1, $chwr_ddc
# CHECK-SAME: encoding: [0x48,0x01,0x03,0x7f]
CReadHwr $c1, $31
# CHECK: creadhwr	$c1, $chwr_epcc
# CHECK-SAME: encoding: [0x48,0x01,0xfb,0x7f]

# 0x3bf = ﻿hex(0x3f + (0xe << 6))
CWriteHwr $c1, $0
# CHECK: cwritehwr	$c1, $chwr_ddc
# CHECK-SAME: encoding: [0x48,0x01,0x03,0xbf]
CWriteHwr $c1, $31
# CHECK: cwritehwr	$c1, $chwr_epcc
# CHECK-SAME: encoding: [0x48,0x01,0xfb,0xbf]

.ifdef BAD
CWriteHwr $c1, $hwr_cpunum  # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, $c2          # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, $32          # ERR: [[@LINE]]:16: error: invalid register number
CWriteHwr $c1, 0            # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, -1           # ERR: [[@LINE]]:16: error: invalid operand for instruction
CWriteHwr $c1, $chwr_foo    # ERR: [[@LINE]]:16: error: invalid operand for instruction
.endif


# Test the symbolic names:
CReadHwr $c1, $chwr_ddc
# CHECK:  creadhwr        $c1, $chwr_ddc  # encoding: [0x48,0x01,0x03,0x7f]
CReadHwr $c1, $chwr_userlocal
# CHECK:  creadhwr        $c1, $chwr_userlocal # encoding: [0x48,0x01,0x0b,0x7f]
CReadHwr $c1, $chwr_priv_userlocal
# CHECK:  creadhwr        $c1, $chwr_priv_userlocal # encoding: [0x48,0x01,0x43,0x7f]
CReadHwr $c1, $chwr_kr1c
#CHECK:   creadhwr        $c1, $chwr_kr1c # encoding: [0x48,0x01,0xb3,0x7f]
CReadHwr $c1, $chwr_kr2c
# CHECK:  creadhwr        $c1, $chwr_kr2c # encoding: [0x48,0x01,0xbb,0x7f]
CReadHwr $c1, $chwr_kcc
# CHECK:  creadhwr        $c1, $chwr_kcc  # encoding: [0x48,0x01,0xeb,0x7f]
CReadHwr $c1, $chwr_kdc
# CHECK:  creadhwr        $c1, $chwr_kdc  # encoding: [0x48,0x01,0xf3,0x7f]
CReadHwr $c1, $chwr_epcc
# CHECK:  creadhwr        $c1, $chwr_epcc # encoding: [0x48,0x01,0xfb,0x7f]


