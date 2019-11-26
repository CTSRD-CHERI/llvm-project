; RUNs: opt -O3 -S %s -o - -print-after-all
; RUN: opt -codegenprepare -S %s -o -
; RUN: opt -codegenprepare -S %s -o - | FileCheck %s

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #1
declare void @llvm.assume(i1)

define void @assume_aligned(i8* %dst, i8* nocapture readonly %src) local_unnamed_addr #0 {
entry:
  %ptrint = ptrtoint i8* %dst to i64
  %maskedptr = and i64 %ptrint, 31
  %maskcond = icmp eq i64 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %dst, i8* align 1 %src, i64 32, i1 false)
  ret void
}

define void @assume_aligned_intermediate_store(i8* %dst, i8* %src) #0 {
entry:
  %dst.addr = alloca i8*, align 8
  %src.addr = alloca i8*, align 8
  %aligned = alloca i8*, align 8
  store i8* %dst, i8** %dst.addr, align 8
  store i8* %src, i8** %src.addr, align 8
  %0 = load i8*, i8** %dst.addr, align 8
  %ptrint = ptrtoint i8* %0 to i64
  %maskedptr = and i64 %ptrint, 31
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  store i8* %0, i8** %aligned, align 8
  %1 = load i8*, i8** %aligned, align 8
  %2 = load i8*, i8** %src.addr, align 8
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %1, i8* align 1 %2, i64 32, i1 false)
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind }
