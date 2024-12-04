; RUN: %cheri_llc %s -collect-csetbounds-output=/dev/stderr -collect-csetbounds-stats=csv -filetype=null -o /dev/null 2>&1 | FileCheck %s

; CHECK: alignment_bits,size,kind,source_loc,compiler_pass,details
; CHECK-EMPTY:
%class.a = type { i32 (...)** }

@c = global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @_Z3fn1v() {
entry:
  %co = alloca ptr, align 8
  %0 = load ptr, ptr %co, align 8
  %1 = load i32, ptr @c, align 4
  %vtable = load ptr, ptr %0, align 8
  %2 = load ptr, ptr %vtable, align 8
  %call = call ptr %2(ptr nonnull %0, i32 signext %1)
  store ptr %call, ptr %co, align 8
  ret void
}
