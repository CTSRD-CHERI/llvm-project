; REQUIRES: asserts
; RUN: opt < %s -basic-aa -gvn -stats -disable-output 2>&1 | FileCheck %s
; rdar://7363102
; For some reason this doesn't do the same for CHERI
; XFAIL: *

; CHECK: Number of loads deleted

; GVN should be able to eliminate load %tmp22.i, because it is redundant with
; load %tmp8.i. This requires being able to prove that %tmp7.i doesn't
; alias the malloc'd value %tmp.i20.i.i, which it can do since %tmp7.i
; is derived from %tmp5.i which is computed from a load, and %tmp.i20.i.i
; is never stored and does not escape.

target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
target triple = "mips64-unknown-freebsd"

%"struct.llvm::MallocAllocator" = type <{ i8 }>
%"struct.llvm::StringMap<void*,llvm::MallocAllocator>" = type { %"struct.llvm::StringMapImpl", %"struct.llvm::MallocAllocator" }
%"struct.llvm::StringMapEntry<void*>" = type { %"struct.llvm::StringMapEntryBase", ptr addrspace(200) }
%"struct.llvm::StringMapEntryBase" = type { i32 }
%"struct.llvm::StringMapImpl" = type { ptr addrspace(200), i32, i32, i32, i32 }
%"struct.llvm::StringMapImpl::ItemBucket" = type { i32, ptr addrspace(200) }
%"struct.llvm::StringRef" = type { ptr addrspace(200), i64 }

define ptr addrspace(200) @_Z3fooRN4llvm9StringMapIPvNS_15MallocAllocatorEEEPKc(ptr addrspace(200) %X, ptr addrspace(200) %P) ssp {
entry:
  %tmp = alloca %"struct.llvm::StringRef", align 16, addrspace(200)
  %tmp.i = getelementptr inbounds %"struct.llvm::StringRef", ptr addrspace(200) %tmp, i64 0, i32 0
  store ptr addrspace(200) %P, ptr addrspace(200) %tmp.i, align 16
  %tmp1.i = call i64 @strlen(ptr addrspace(200) %P) nounwind readonly
  %tmp2.i = getelementptr inbounds %"struct.llvm::StringRef", ptr addrspace(200) %tmp, i64 0, i32 1
  store i64 %tmp1.i, ptr addrspace(200) %tmp2.i, align 8
  %tmp1 = call ptr addrspace(200) @_ZN4llvm9StringMapIPvNS_15MallocAllocatorEE16GetOrCreateValueERKNS_9StringRefE(ptr addrspace(200) %X, ptr addrspace(200) %tmp) ssp
  ret ptr addrspace(200) %tmp1
}

declare i64 @strlen(ptr addrspace(200) nocapture) nounwind readonly

declare noalias ptr addrspace(200) @malloc(i64) nounwind

declare i32 @_ZN4llvm13StringMapImpl15LookupBucketForENS_9StringRefE(ptr addrspace(200), i64, i64)

define linkonce_odr ptr addrspace(200) @_ZN4llvm9StringMapIPvNS_15MallocAllocatorEE16GetOrCreateValueERKNS_9StringRefE(ptr addrspace(200) %this, ptr addrspace(200) nocapture %Key) ssp align 2 {
entry:
  %val = load i64, ptr addrspace(200) %Key
  %tmp = getelementptr inbounds %"struct.llvm::StringRef", ptr addrspace(200) %Key, i64 0, i32 1
  %val2 = load i64, ptr addrspace(200) %tmp
  %tmp2.i = getelementptr inbounds %"struct.llvm::StringMap<void*,llvm::MallocAllocator>", ptr addrspace(200) %this, i64 0, i32 0
  %tmp3.i = tail call i32 @_ZN4llvm13StringMapImpl15LookupBucketForENS_9StringRefE(ptr addrspace(200) %tmp2.i, i64 %val, i64 %val2)
  %tmp4.i = getelementptr inbounds %"struct.llvm::StringMap<void*,llvm::MallocAllocator>", ptr addrspace(200) %this, i64 0, i32 0, i32 0
  %tmp5.i = load ptr addrspace(200), ptr addrspace(200) %tmp4.i, align 16
  %tmp6.i = zext i32 %tmp3.i to i64
  %tmp7.i = getelementptr inbounds %"struct.llvm::StringMapImpl::ItemBucket", ptr addrspace(200) %tmp5.i, i64 %tmp6.i, i32 1
  %tmp8.i = load ptr addrspace(200), ptr addrspace(200) %tmp7.i, align 16
  %tmp9.i = icmp eq ptr addrspace(200) %tmp8.i, null
  %tmp13.i = icmp eq ptr addrspace(200) %tmp8.i, inttoptr (i64 -1 to ptr addrspace(200))
  %or.cond.i = or i1 %tmp9.i, %tmp13.i
  br i1 %or.cond.i, label %bb4.i, label %bb6.i

bb4.i:                                            ; preds = %entry
  %tmp41.i = inttoptr i64 %val to ptr addrspace(200)
  %tmp4.i35.i = getelementptr inbounds i8, ptr addrspace(200) %tmp41.i, i64 %val2
  %tmp.i.i = ptrtoint ptr addrspace(200) %tmp4.i35.i to i64
  %tmp1.i.i = trunc i64 %tmp.i.i to i32
  %tmp3.i.i = trunc i64 %val to i32
  %tmp4.i.i = sub i32 %tmp1.i.i, %tmp3.i.i
  %tmp5.i.i = add i32 %tmp4.i.i, 17
  %tmp8.i.i = zext i32 %tmp5.i.i to i64
  %tmp.i20.i.i = tail call noalias ptr addrspace(200) @malloc(i64 %tmp8.i.i) nounwind
  %tmp12.i.i = icmp eq ptr addrspace(200) %tmp.i20.i.i, null
  br i1 %tmp12.i.i, label %_ZN4llvm14StringMapEntryIPvE6CreateINS_15MallocAllocatorES1_EEPS2_PKcS7_RT_T0_.exit.i, label %bb.i.i

bb.i.i:                                           ; preds = %bb4.i
  store i32 %tmp4.i.i, ptr addrspace(200) %tmp.i20.i.i, align 4
  %tmp1.i19.i.i = getelementptr inbounds i8, ptr addrspace(200) %tmp.i20.i.i, i64 8
  store ptr addrspace(200) null, ptr addrspace(200) %tmp1.i19.i.i, align 16
  br label %_ZN4llvm14StringMapEntryIPvE6CreateINS_15MallocAllocatorES1_EEPS2_PKcS7_RT_T0_.exit.i

_ZN4llvm14StringMapEntryIPvE6CreateINS_15MallocAllocatorES1_EEPS2_PKcS7_RT_T0_.exit.i: ; preds = %bb.i.i, %bb4.i
  %tmp.i18.i.i = getelementptr inbounds i8, ptr addrspace(200) %tmp.i20.i.i, i64 16
  %tmp15.i.i = zext i32 %tmp4.i.i to i64
  tail call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) %tmp.i18.i.i, ptr addrspace(200) %tmp41.i, i64 %tmp15.i.i, i1 false)
  %tmp.i18.sum.i.i = add i64 %tmp15.i.i, 16
  %tmp17.i.i = getelementptr inbounds i8, ptr addrspace(200) %tmp.i20.i.i, i64 %tmp.i18.sum.i.i
  store i8 0, ptr addrspace(200) %tmp17.i.i, align 1
  %tmp.i.i.i = getelementptr inbounds i8, ptr addrspace(200) %tmp.i20.i.i, i64 8
  store ptr addrspace(200) null, ptr addrspace(200) %tmp.i.i.i, align 16
  %tmp22.i = load ptr addrspace(200), ptr addrspace(200) %tmp7.i, align 16
  %tmp24.i = icmp eq ptr addrspace(200) %tmp22.i, inttoptr (i64 -1 to ptr addrspace(200))
  br i1 %tmp24.i, label %bb9.i, label %_ZN4llvm9StringMapIPvNS_15MallocAllocatorEE16GetOrCreateValueIS1_EERNS_14StringMapEntryIS1_EENS_9StringRefET_.exit

bb6.i:                                            ; preds = %entry
  ret ptr addrspace(200) %tmp8.i

bb9.i:                                            ; preds = %_ZN4llvm14StringMapEntryIPvE6CreateINS_15MallocAllocatorES1_EEPS2_PKcS7_RT_T0_.exit.i
  %tmp25.i = getelementptr inbounds %"struct.llvm::StringMap<void*,llvm::MallocAllocator>", ptr addrspace(200) %this, i64 0, i32 0, i32 3
  %tmp26.i = load i32, ptr addrspace(200) %tmp25.i, align 8
  %tmp27.i = add i32 %tmp26.i, -1
  store i32 %tmp27.i, ptr addrspace(200) %tmp25.i, align 8
  ret ptr addrspace(200) %tmp.i20.i.i

_ZN4llvm9StringMapIPvNS_15MallocAllocatorEE16GetOrCreateValueIS1_EERNS_14StringMapEntryIS1_EENS_9StringRefET_.exit: ; preds = %_ZN4llvm14StringMapEntryIPvE6CreateINS_15MallocAllocatorES1_EEPS2_PKcS7_RT_T0_.exit.i
  ret ptr addrspace(200) %tmp.i20.i.i
}

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) nocapture, ptr addrspace(200) nocapture, i64, i1) nounwind
