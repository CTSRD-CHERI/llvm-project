; RUN: opt -passes=globalopt -S -o - < %s | FileCheck %s
; This triggers an assertion in llvm::optimizeGlobalCtorsList()
; due to using getPointerAddressSpace() on a non-pointer type.
target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"
target triple = "riscv64-unknown-freebsd"

@llvm.global_ctors = appending addrspace(200) global [5 x { i32, ptr addrspace(200), ptr addrspace(200) }] [
  { i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @__cxx_global_var_init.1, ptr addrspace(200) null },
  { i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @__cxx_global_var_init.2, ptr addrspace(200) null },
  { i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @__cxx_global_var_init.3, ptr addrspace(200) null },
  { i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @__cxx_global_var_init.4, ptr addrspace(200) null },
  { i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @__cxx_global_var_init.5, ptr addrspace(200) null }
]

@global_i64 = addrspace(200) global i64 0

@global_double = addrspace(200) global double undef
; CHECK: @global_double = local_unnamed_addr addrspace(200) global double 2.470330e-323

@global_double2 = addrspace(200) global double undef
; CHECK: @global_double2 = local_unnamed_addr addrspace(200) global double 2.964390e-323

@global_double_cap = addrspace(200) global ptr addrspace(200) undef
; CHECK: @global_double_cap = local_unnamed_addr addrspace(200) global ptr addrspace(200) @global_i64

@global_double_ptr1 = addrspace(200) global ptr undef
; CHECK: @global_double_ptr1 = local_unnamed_addr addrspace(200) global ptr addrspacecast (ptr addrspace(200) @global_i64 to ptr)

@global_double_ptr2 = addrspace(200) global ptr undef
; CHECK: @global_double_ptr2 = local_unnamed_addr addrspace(200) global ptr addrspacecast (ptr addrspace(200) null to ptr)

define internal void @__cxx_global_var_init.1() addrspace(200) nounwind {
entry:
  %call1 = call double @i64_to_double(i64 5)
  store double %call1, ptr addrspace(200) @global_double, align 8
  ret void
}
define internal void @__cxx_global_var_init.2() addrspace(200) nounwind {
entry:
  %call1 = call double @i64_to_double_with_alloca(i64 6)
  store double %call1, ptr addrspace(200) @global_double2, align 8
  ret void
}
define internal void @__cxx_global_var_init.3() addrspace(200) nounwind {
entry:
  %call = call ptr addrspace(200) @i64cap_to_doublecap(ptr addrspace(200) @global_i64)
  store ptr addrspace(200) %call, ptr addrspace(200) @global_double_cap, align 16
  ret void
}
define internal void @__cxx_global_var_init.4() addrspace(200) nounwind {
entry:
  %call = call ptr @i64cap_to_doubleptr(ptr addrspace(200) @global_i64)
  store ptr %call, ptr addrspace(200) @global_double_ptr1, align 8
  ret void
}

define internal void @__cxx_global_var_init.5() addrspace(200) nounwind {
entry:
  %call = call ptr @null_i64cap_to_doubleptr()
  store ptr %call, ptr addrspace(200) @global_double_ptr2, align 8
  ret void
}

define internal double @i64_to_double(i64 %e) addrspace(200) {
entry:
  %0 = bitcast i64 %e to double
  ret double %0
}

define internal double @i64_to_double_with_alloca(i64 %e) addrspace(200) {
entry:
  %alloc = alloca double, align 8, addrspace(200)
  %tmp = bitcast ptr addrspace(200) %alloc to ptr addrspace(200)
  store i64 %e, ptr addrspace(200) %tmp, align 8
  %result = load double, ptr addrspace(200) %alloc, align 8
  ret double %result
}

define internal ptr addrspace(200) @i64cap_to_doublecap(ptr addrspace(200) %e) addrspace(200) {
entry:
  %0 = bitcast ptr addrspace(200) %e to ptr addrspace(200)
  ret ptr addrspace(200) %0
}

define internal ptr @i64cap_to_doubleptr(ptr addrspace(200) %e) addrspace(200) {
entry:
  %0 = bitcast ptr addrspace(200) %e to ptr addrspace(200)
  %1 = addrspacecast ptr addrspace(200) %0 to ptr
  ret ptr %1
}

define internal ptr @null_i64cap_to_doubleptr() addrspace(200) {
entry:
  %0 = bitcast ptr addrspacecast (ptr addrspace(200) null to ptr) to ptr
  ret ptr %0
}
