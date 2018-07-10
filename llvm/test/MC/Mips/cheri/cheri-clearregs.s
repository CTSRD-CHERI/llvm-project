# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
#
# Check that the assembler is able to handle clear register instructions.
#

ClearLo 0x1234
# CHECK: clearlo	4660                    # encoding: [0x49,0xe0,0x12,0x34]
ClearHi 0x1234
# CHECK: clearhi	4660                    # encoding: [0x49,0xe1,0x12,0x34]
CClearLo 0x1234
# CHECK: cclearlo	4660            # encoding: [0x49,0xe2,0x12,0x34]
CClearHi 0x1234
# CHECK: cclearhi	4660            # encoding: [0x49,0xe3,0x12,0x34]
