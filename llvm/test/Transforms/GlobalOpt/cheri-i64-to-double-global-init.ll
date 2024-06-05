; RUN: opt -passes=globalopt -S -o - < %s | FileCheck %s
; This triggers an assertion in llvm::optimizeGlobalCtorsList()
; due to using getPointerAddressSpace() on a non-pointer type.
target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"
target triple = "riscv64-unknown-freebsd"

@llvm.global_ctors = appending addrspace(200) global [5 x { i32, void () addrspace(200)*, i8 addrspace(200)* }] [
     { i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @__cxx_global_var_init.1, i8 addrspace(200)* null },
     { i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @__cxx_global_var_init.2, i8 addrspace(200)* null },
     { i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @__cxx_global_var_init.3, i8 addrspace(200)* null },
     { i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @__cxx_global_var_init.4, i8 addrspace(200)* null },
     { i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @__cxx_global_var_init.5, i8 addrspace(200)* null }
]

@global_i64 = addrspace(200) global i64 0

@global_double = addrspace(200) global double undef
; CHECK: @global_double = local_unnamed_addr addrspace(200) global double 2.470330e-323

@global_double2 = addrspace(200) global double undef
; CHECK: @global_double2 = local_unnamed_addr addrspace(200) global double 2.964390e-323

@global_double_cap = addrspace(200) global double addrspace(200)* undef
; CHECK: @global_double_cap = local_unnamed_addr addrspace(200) global double addrspace(200)* bitcast (i64 addrspace(200)* @global_i64 to double addrspace(200)*)

@global_double_ptr1 = addrspace(200) global double addrspace(0)* undef
; CHECK: @global_double_ptr1 = local_unnamed_addr addrspace(200) global double* addrspacecast (double addrspace(200)* bitcast (i64 addrspace(200)* @global_i64 to double addrspace(200)*) to double*)

@global_double_ptr2 = addrspace(200) global double addrspace(0)* undef
; CHECK: @global_double_ptr2 = local_unnamed_addr addrspace(200) global double* addrspacecast (double addrspace(200)* null to double*)

define internal void @__cxx_global_var_init.1() addrspace(200) nounwind {
entry:
  %call1 = call double @i64_to_double(i64 5)
  store double %call1, double addrspace(200)* @global_double
  ret void
}
define internal void @__cxx_global_var_init.2() addrspace(200) nounwind {
entry:
  %call1 = call double @i64_to_double_with_alloca(i64 6)
  store double %call1, double addrspace(200)* @global_double2
  ret void
}
define internal void @__cxx_global_var_init.3() addrspace(200) nounwind {
entry:
  %call = call double addrspace(200)* @i64cap_to_doublecap(i64 addrspace(200)* @global_i64)
  store double addrspace(200)* %call, double addrspace(200)* addrspace(200)* @global_double_cap
  ret void
}
define internal void @__cxx_global_var_init.4() addrspace(200) nounwind {
entry:
  %call = call double* @i64cap_to_doubleptr(i64 addrspace(200)* @global_i64)
  store double addrspace(0)* %call, double addrspace(0)* addrspace(200)* @global_double_ptr1
  ret void
}

define internal void @__cxx_global_var_init.5() addrspace(200) nounwind {
entry:
  %call = call double* @null_i64cap_to_doubleptr()
  store double addrspace(0)* %call, double addrspace(0)* addrspace(200)* @global_double_ptr2
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
  %tmp = bitcast double addrspace(200)* %alloc to i64 addrspace(200)*
  store i64 %e, i64 addrspace(200)* %tmp
  %result = load double, double addrspace(200)* %alloc
  ret double %result
}

define internal double addrspace(200)* @i64cap_to_doublecap(i64 addrspace(200)* %e) addrspace(200) {
entry:
  %0 = bitcast i64 addrspace(200)* %e to double addrspace(200)*
  ret double addrspace(200)* %0
}

define internal double addrspace(0)* @i64cap_to_doubleptr(i64 addrspace(200)* %e) addrspace(200) {
entry:
  %0 = bitcast i64 addrspace(200)* %e to double addrspace(200)*
  %1 = addrspacecast double addrspace(200)* %0 to double addrspace(0)*
  ret double addrspace(0)* %1
}

define internal double addrspace(0)* @null_i64cap_to_doubleptr() addrspace(200) {
entry:
  %0 = bitcast i64 addrspace(0)* addrspacecast (i64 addrspace(200)* null to i64 addrspace(0)*) to double addrspace(0)*
  ret double addrspace(0)* %0
}

attributes #0 = { nounwind }
