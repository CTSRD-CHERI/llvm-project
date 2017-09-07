; RUN: not llvm-as < %s 2>&1 | FileCheck %s

target datalayout = "A1"

; CHECK: :7:41: error: alloca address space 2 must match datalayout AS 1
define void @use_alloca() {
  %alloca_scalar_no_align = alloca i32, addrspace(2)
  ret void
}
