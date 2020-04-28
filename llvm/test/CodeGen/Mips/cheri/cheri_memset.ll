; RUN: %cheri_llc -relocation-model=pic %s -o - | FileCheck %s
; ModuleID = 'memset.c'
%struct.x = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }

@blob = common global %struct.x zeroinitializer, align 4
@blob2 = common global %struct.x zeroinitializer, align 4

declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #1

define void @zero() nounwind {
entry:
; CHECK: (memset_c)
; CHECK: jalr
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* addrspacecast (%struct.x* @blob to %struct.x addrspace(200)*) to i8 addrspace(200)*), i8 0, i64 40, i32 4, i1 false)
; This one doesn't get inlined anymore, because it's more than one capability in size.
; CHECK: (memcpy_c)
; CHECK: jalr
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* addrspacecast (%struct.x* @blob2 to %struct.x addrspace(200)*) to i8 addrspace(200)*), i8 addrspace(200)* bitcast (%struct.x addrspace(200)* addrspacecast (%struct.x* @blob to %struct.x addrspace(200)*) to i8 addrspace(200)*), i64 40, i32 4, i1 false)
; CHECK: (memmove_c)
; CHECK: jalr
  tail call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* bitcast (%struct.x addrspace(200)* addrspacecast (%struct.x* @blob2 to %struct.x addrspace(200)*) to i8 addrspace(200)*), i8 addrspace(200)* bitcast (%struct.x addrspace(200)* addrspacecast (%struct.x* @blob to %struct.x addrspace(200)*) to i8 addrspace(200)*), i64 40, i32 4, i1 false)
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
