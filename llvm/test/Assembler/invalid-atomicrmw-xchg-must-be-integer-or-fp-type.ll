; RUN: not llvm-as -disable-output %s 2>&1 | FileCheck %s

; CHECK: error: atomicrmw xchg operand must be an integer, pointer or floating point type
define void @f([2 x i32]* %ptr) {
  atomicrmw xchg [2 x i32]* %ptr, [2 x i32] [i32 0, i32 0] seq_cst
  ret void
}
