# RUN: %cheri_llvm-mc %s -show-encoding -o - | FileCheck %s

.text
cgetpcc $c1
# CHECK: cgetpcc $c1 # encoding: [0x48,0x01,0x07,0xff]
cgetpccsetoffset $c1, $2
# CHECK-NEXT: cgetpccsetoffset $c1, $2 # encoding: [0x48,0x01,0x11,0xff]
cgetpccincoffset $c1, $2
# CHECK-NEXT: cgetpccincoffset $c1, $2 # encoding: [0x48,0x01,0x14,0xff]
cgetpccsetaddr $c1, $2
# CHECK-NEXT: cgetpccsetaddr $c1, $2 # encoding: [0x48,0x01,0x15,0x3f]
