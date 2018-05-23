; RUN: %cheri_llc -relocation-model=pic %s -o - | FileCheck %s
; ModuleID = 'memset.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.x = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }

@blob = common addrspace(200) global %struct.x zeroinitializer, align 4
@blob2 = common addrspace(200) global %struct.x zeroinitializer, align 4

declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #1


; Function Attrs: nounwind
define void @zero() #0 {
entry:
; CHECK: memset_c
; CHECK: jalr
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* @blob to i8 addrspace(200)*), i8 0, i64 40, i32 4, i1 false)
; This one doesn't get inlined anymore, because it's more than one capability in size.
; CHECK: memcpy_c
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* @blob2 to i8 addrspace(200)*), i8 addrspace(200)* bitcast (%struct.x addrspace(200)* @blob to i8 addrspace(200)*), i64 40, i32 4, i1 false)
; CHECK: memmove_c
; CHECK: jalr
  tail call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* @blob2 to i8 addrspace(200)*), i8 addrspace(200)* bitcast (%struct.x addrspace(200)* @blob to i8 addrspace(200)*), i64 40, i32 4, i1 false)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture, i8, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
