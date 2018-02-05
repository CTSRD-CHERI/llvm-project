; RUN: %cheri_purecap_llc -O0 %s -o -
; Reduced test case for index.c no longer compiling after memset optimization
; See https://github.com/CTSRD-CHERI/llvm/issues/265
%struct.am = type { %struct.n, %struct.m, [8 x i8] }
%struct.n = type { %struct.n addrspace(200)* }
%struct.m = type { i32, i32, i8, i8, i8, i8, i8, i32, i32 }

@q = common addrspace(200) global %struct.am zeroinitializer, align 16
@p = common addrspace(200) global %struct.am zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone
define void @r() #0 {
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 bitcast (%struct.m addrspace(200)* getelementptr inbounds (%struct.am, %struct.am addrspace(200)* @q, i32 0, i32 1) to i8 addrspace(200)*), i8 addrspace(200)* align 16 bitcast (%struct.m addrspace(200)* getelementptr inbounds (%struct.am, %struct.am addrspace(200)* @p, i32 0, i32 1) to i8 addrspace(200)*), i64 24, i1 false)
; Check that we do a 24-byte copy as a capability load/store followed by a double load / store
; CHECK: clc
; CHECK: csc
; CHECK: cld
; CHECK: csd

  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1) #1

attributes #0 = { noinline nounwind optnone }
attributes #1 = { argmemonly nounwind }
