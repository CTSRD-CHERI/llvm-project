# RUN: llvm-mc %s -triple=mipsel-unknown-linux -mcpu=mips1 2>&1 | FileCheck %s
.set bopt
# CHECK: warning: 'bopt' feature is unsupported
.set nobopt # should be ignored
