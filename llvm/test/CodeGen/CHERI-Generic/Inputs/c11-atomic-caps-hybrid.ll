@IF-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - -mattr=+a | FileCheck %s --check-prefixes=CHECK,ATOMICS
@IF-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - -mattr=-a | FileCheck %s --check-prefixes=CHECK,LIBCALLS
@IFNOT-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - | FileCheck %s
; target datalayout = "@HYBRID_DATALAYOUT@"

; Function Attrs: nofree norecurse nounwind
define i8 addrspace(200)* @test_load(i8 addrspace(200)** nocapture readonly %f) local_unnamed_addr #0 {
entry:
  %0 = load atomic i8 addrspace(200)*, i8 addrspace(200)** %f seq_cst, align 16
  ret i8 addrspace(200)* %0
}

; Function Attrs: nofree norecurse nounwind
define void @test_store(i8 addrspace(200)** nocapture %f, i8 addrspace(200)* %value) local_unnamed_addr #0 {
entry:
  store atomic i8 addrspace(200)* %value, i8 addrspace(200)** %f seq_cst, align 16
  ret void
}

; Function Attrs: nofree norecurse nounwind writeonly
define void @test_init(i8 addrspace(200)** nocapture %f, i8 addrspace(200)* %value) local_unnamed_addr #1 {
entry:
  store i8 addrspace(200)* %value, i8 addrspace(200)** %f, align 16
  ret void
}

; Function Attrs: nofree norecurse nounwind
define i8 addrspace(200)* @test_xchg(i8 addrspace(200)** nocapture %f, i8 addrspace(200)* %value) local_unnamed_addr #0 {
entry:
  %0 = atomicrmw xchg i8 addrspace(200)** %f, i8 addrspace(200)* %value seq_cst
  ret i8 addrspace(200)* %0
}

; Function Attrs: nofree norecurse nounwind
define i64 addrspace(200)* @test_xchg_long_ptr(i64 addrspace(200)** nocapture %f, i64 addrspace(200)* %value) local_unnamed_addr #0 {
entry:
  %0 = bitcast i64 addrspace(200)** %f to i8 addrspace(200)**
  %1 = bitcast i64 addrspace(200)* %value to i8 addrspace(200)*
  %2 = atomicrmw xchg i8 addrspace(200)** %0, i8 addrspace(200)* %1 seq_cst
  %3 = bitcast i8 addrspace(200)* %2 to i64 addrspace(200)*
  ret i64 addrspace(200)* %3
}

; Function Attrs: nofree norecurse nounwind
define zeroext i1 @test_cmpxchg_weak(i8 addrspace(200)** nocapture %f, i8 addrspace(200)** nocapture %exp, i8 addrspace(200)* %new) local_unnamed_addr #0 {
entry:
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %exp, align 16
  %1 = cmpxchg weak i8 addrspace(200)** %f, i8 addrspace(200)* %0, i8 addrspace(200)* %new monotonic monotonic
  %2 = extractvalue { i8 addrspace(200)*, i1 } %1, 1
  br i1 %2, label %cmpxchg.continue, label %cmpxchg.store_expected

cmpxchg.store_expected:                           ; preds = %entry
  %3 = extractvalue { i8 addrspace(200)*, i1 } %1, 0
  store i8 addrspace(200)* %3, i8 addrspace(200)** %exp, align 16
  br label %cmpxchg.continue

cmpxchg.continue:                                 ; preds = %cmpxchg.store_expected, %entry
  ret i1 %2
}

; Function Attrs: nofree norecurse nounwind
define zeroext i1 @test_cmpxchg_strong(i8 addrspace(200)** nocapture %f, i8 addrspace(200)** nocapture %exp, i8 addrspace(200)* %new) local_unnamed_addr #0 {
entry:
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %exp, align 16
  %1 = cmpxchg i8 addrspace(200)** %f, i8 addrspace(200)* %0, i8 addrspace(200)* %new monotonic monotonic
  %2 = extractvalue { i8 addrspace(200)*, i1 } %1, 1
  br i1 %2, label %cmpxchg.continue, label %cmpxchg.store_expected

cmpxchg.store_expected:                           ; preds = %entry
  %3 = extractvalue { i8 addrspace(200)*, i1 } %1, 0
  store i8 addrspace(200)* %3, i8 addrspace(200)** %exp, align 16
  br label %cmpxchg.continue

cmpxchg.continue:                                 ; preds = %cmpxchg.store_expected, %entry
  ret i1 %2
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_add_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_add_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_add_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i64 addrspace(200)* @test_fetch_add_longptr(i64 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)* %value)
  %1 = shl iCAPRANGE %0, 3
  %2 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %1
  %3 = bitcast i64 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_add_cap(i8* %3, i8 addrspace(200)* %2, i32 signext 5) #4
  %4 = bitcast i8 addrspace(200)* %call to i64 addrspace(200)*
  ret i64 addrspace(200)* %4
}

; Function Attrs: nounwind readnone willreturn
declare iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)*) #3

; Function Attrs: nounwind
define i64 addrspace(200)* @test_fetch_add_longptr_and_short(i64 addrspace(200)** %ptr, i16 signext %value) local_unnamed_addr #2 {
entry:
  %conv = sext i16 %value to iCAPRANGE
  %0 = shl nsw iCAPRANGE %conv, 3
  %1 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %0
  %2 = bitcast i64 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_add_cap(i8* %2, i8 addrspace(200)* %1, i32 signext 5) #4
  %3 = bitcast i8 addrspace(200)* %call to i64 addrspace(200)*
  ret i64 addrspace(200)* %3
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_add_charptr(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)* %value)
  %1 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %0
  %2 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_add_cap(i8* %2, i8 addrspace(200)* %1, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_add_charptr_and_short(i8 addrspace(200)** %ptr, i16 signext %value) local_unnamed_addr #2 {
entry:
  %conv = sext i16 %value to iCAPRANGE
  %0 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %conv
  %1 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_add_cap(i8* %1, i8 addrspace(200)* %0, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_sub_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_sub_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i64 addrspace(200)* @test_fetch_sub_longptr(i64 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)* %value)
  %1 = shl iCAPRANGE %0, 3
  %2 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %1
  %3 = bitcast i64 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8* %3, i8 addrspace(200)* %2, i32 signext 5) #4
  %4 = bitcast i8 addrspace(200)* %call to i64 addrspace(200)*
  ret i64 addrspace(200)* %4
}

; Function Attrs: nounwind
define i64 addrspace(200)* @test_fetch_sub_longptr_and_short(i64 addrspace(200)** %ptr, i16 signext %value) local_unnamed_addr #2 {
entry:
  %conv = sext i16 %value to iCAPRANGE
  %0 = shl nsw iCAPRANGE %conv, 3
  %1 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %0
  %2 = bitcast i64 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8* %2, i8 addrspace(200)* %1, i32 signext 5) #4
  %3 = bitcast i8 addrspace(200)* %call to i64 addrspace(200)*
  ret i64 addrspace(200)* %3
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_sub_charptr(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)* %value)
  %1 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %0
  %2 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8* %2, i8 addrspace(200)* %1, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_sub_charptr_and_short(i8 addrspace(200)** %ptr, i16 signext %value) local_unnamed_addr #2 {
entry:
  %conv = sext i16 %value to iCAPRANGE
  %0 = getelementptr i8, i8 addrspace(200)* null, iCAPRANGE %conv
  %1 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8* %1, i8 addrspace(200)* %0, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_and_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_and_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_and_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_or_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_or_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_or_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_xor_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_xor_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_xor_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_max_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_umax_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_umax_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr

; Function Attrs: nounwind
define i8 addrspace(200)* @test_fetch_min_uintcap(i8 addrspace(200)** %ptr, i8 addrspace(200)* %value) local_unnamed_addr #2 {
entry:
  %0 = bitcast i8 addrspace(200)** %ptr to i8*
  %call = call i8 addrspace(200)* @__atomic_fetch_umin_cap(i8* %0, i8 addrspace(200)* %value, i32 signext 5) #4
  ret i8 addrspace(200)* %call
}

declare i8 addrspace(200)* @__atomic_fetch_umin_cap(i8*, i8 addrspace(200)*, i32) local_unnamed_addr
