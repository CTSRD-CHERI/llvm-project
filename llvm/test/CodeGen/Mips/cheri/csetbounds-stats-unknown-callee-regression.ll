; RUN: %cheri_llc %s -collect-csetbounds-output=/dev/stderr -collect-csetbounds-stats=csv -filetype=null -o /dev/null 2>&1 | FileCheck %s

; CHECK: alignment_bits,size,kind,source_loc,compiler_pass,details
; CHECK-EMPTY:
%class.a = type { i32 (...)** }

@c = global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @_Z3fn1v() {
entry:
  %co = alloca %class.a*, align 8
  %0 = load %class.a*, %class.a** %co, align 8
  %1 = load i32, i32* @c, align 4
  %2 = bitcast %class.a* %0 to i8* (%class.a*, i32)***
  %vtable = load i8* (%class.a*, i32)**, i8* (%class.a*, i32)*** %2, align 8
  %vfn = getelementptr inbounds i8* (%class.a*, i32)*, i8* (%class.a*, i32)** %vtable, i64 0
  %3 = load i8* (%class.a*, i32)*, i8* (%class.a*, i32)** %vfn, align 8
  %call = call i8* %3(%class.a* %0, i32 signext %1)
  %4 = bitcast i8* %call to %class.a*
  store %class.a* %4, %class.a** %co, align 8
  ret void
}
