# RUN: llvm-mc %s -triple=mipsel-unknown-linux -mcpu=mips1 2>%t1
# RUN: FileCheck %s < %t1
.set bopt
# CHECK: warning: branch optimisation is not supported
.set nobopt # should be ignored
