# RUN: echo "0xef 0xef 0xef 0xef" | llvm-mc -disassemble -triple=mips64-unknown-linux-gnu | FileCheck %s
# CHECK: .text
# CHECK-NEXT: trap_ri
