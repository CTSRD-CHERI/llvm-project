; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s -o - | FileCheck %s
; Check that the copy from the zeroinitializer global is turned into a series of zero stores
; or memset() as long as the memcpy is not volatile:

%struct.umutex = type { i32, i32, [2 x i32], i8 addrspace(200)*, i32, [2 x i32] }

@_thr_umutex_init.default_mtx = internal addrspace(200) constant %struct.umutex zeroinitializer, align 16

define void @_thr_umutex_init(%struct.umutex addrspace(200)* %mtx) local_unnamed_addr addrspace(200) nounwind "frame-pointer"="none" {
  %1 = bitcast %struct.umutex addrspace(200)* %mtx to i8 addrspace(200)*
  tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %1, i8 addrspace(200)* align 16 bitcast (%struct.umutex addrspace(200)* @_thr_umutex_init.default_mtx to i8 addrspace(200)*), i64 48, i1 false)
  ret void
}

define void @_thr_umutex_init_volatile(%struct.umutex addrspace(200)* %mtx) local_unnamed_addr addrspace(200) nounwind "frame-pointer"="none" {
  %1 = bitcast %struct.umutex addrspace(200)* %mtx to i8 addrspace(200)*
  tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %1, i8 addrspace(200)* align 16 bitcast (%struct.umutex addrspace(200)* @_thr_umutex_init.default_mtx to i8 addrspace(200)*), i64 48, i1 true)
  ret void
}

declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noalias nocapture writeonly %0, i8 addrspace(200)* noalias nocapture readonly %1, i64 %2, i1 immarg %3) addrspace(200)
