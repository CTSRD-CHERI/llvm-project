# RUN: not llvm-mc -triple mips64-unknown-linux-gnu -show-inst %s 2>&1 | FileCheck %s
# https://github.com/CTSRD-CHERI/llvm-project/issues/303

dli $t0, a
# CHECK: [[@LINE-1]]:1: error: could not evaluate operand as immediate

