; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s -o - | FileCheck %s
; Check that the copy from the zeroinitializer global is turned into a series of zero stores
; or memset() as long as the memcpy is not volatile:

%struct.umutex = type { i32, i32, [2 x i32], ptr addrspace(200), i32, [2 x i32] }

@_thr_umutex_init.default_mtx = internal addrspace(200) constant %struct.umutex zeroinitializer, align 16

define void @_thr_umutex_init(ptr addrspace(200) %mtx) local_unnamed_addr addrspace(200) nounwind "frame-pointer"="none" {
  tail call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %mtx, ptr addrspace(200) align 16 @_thr_umutex_init.default_mtx, i64 48, i1 false)
  ret void
}

define void @_thr_umutex_init_volatile(ptr addrspace(200) %mtx) local_unnamed_addr addrspace(200) nounwind "frame-pointer"="none" {
  tail call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %mtx, ptr addrspace(200) align 16 @_thr_umutex_init.default_mtx, i64 48, i1 true)
  ret void
}

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) noalias nocapture writeonly %0, ptr addrspace(200) noalias nocapture readonly %1, i64 %2, i1 immarg %3) addrspace(200)
