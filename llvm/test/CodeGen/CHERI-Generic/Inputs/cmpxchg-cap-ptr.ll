; Check that we can generate sensible code for atomic operations using capability pointers on capabilities
; in both hybrid and purecap mode.
; See https://github.com/CTSRD-CHERI/llvm-project/issues/470
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=PURECAP
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=HYBRID

define { i8, i1 } @test_cmpxchg_strong_i8(i8 addrspace(200)* %ptr, i8 %exp, i8 %new) nounwind {
  %1 = cmpxchg i8 addrspace(200)* %ptr, i8 %exp, i8 %new acq_rel acquire
  ret { i8, i1 } %1
}

define { i16, i1 } @test_cmpxchg_strong_i16(i16 addrspace(200)* %ptr, i16 %exp, i16 %new) nounwind {
  %1 = cmpxchg i16 addrspace(200)* %ptr, i16 %exp, i16 %new acq_rel acquire
  ret { i16, i1 } %1
}

define { i32, i1 } @test_cmpxchg_strong_i32(i32 addrspace(200)* %ptr, i32 %exp, i32 %new) nounwind {
  %1 = cmpxchg i32 addrspace(200)* %ptr, i32 %exp, i32 %new acq_rel acquire
  ret { i32, i1 } %1
}

define { i64, i1 } @test_cmpxchg_strong_i64(i64 addrspace(200)* %ptr, i64 %exp, i64 %new) nounwind {
  %1 = cmpxchg i64 addrspace(200)* %ptr, i64 %exp, i64 %new acq_rel acquire
  ret { i64, i1 } %1
}

define { i8 addrspace(200)*, i1 } @test_cmpxchg_strong_cap(i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %exp, i8 addrspace(200)* %new) nounwind {
  %1 = cmpxchg i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %exp, i8 addrspace(200)* %new acq_rel acquire
  ret { i8 addrspace(200)*, i1 } %1
}

define { i32 addrspace(200)*, i1 } @test_cmpxchg_strong_cap_i32(i32 addrspace(200)* addrspace(200)* %ptr, i32 addrspace(200)* %exp, i32 addrspace(200)* %new) nounwind {
  %1 = cmpxchg weak i32 addrspace(200)* addrspace(200)* %ptr, i32 addrspace(200)* %exp, i32 addrspace(200)* %new acq_rel acquire
  ret { i32 addrspace(200)*, i1 } %1
}


define { i8, i1 } @test_cmpxchg_weak_i8(i8 addrspace(200)* %ptr, i8 %exp, i8 %new) nounwind {
  %1 = cmpxchg weak i8 addrspace(200)* %ptr, i8 %exp, i8 %new acq_rel acquire
  ret { i8, i1 } %1
}

define { i16, i1 } @test_cmpxchg_weak_i16(i16 addrspace(200)* %ptr, i16 %exp, i16 %new) nounwind {
  %1 = cmpxchg weak i16 addrspace(200)* %ptr, i16 %exp, i16 %new acq_rel acquire
  ret { i16, i1 } %1
}

define { i32, i1 } @test_cmpxchg_weak_i32(i32 addrspace(200)* %ptr, i32 %exp, i32 %new) nounwind {
  %1 = cmpxchg weak i32 addrspace(200)* %ptr, i32 %exp, i32 %new acq_rel acquire
  ret { i32, i1 } %1
}

define { i64, i1 } @test_cmpxchg_weak_i64(i64 addrspace(200)* %ptr, i64 %exp, i64 %new) nounwind {
  %1 = cmpxchg weak i64 addrspace(200)* %ptr, i64 %exp, i64 %new acq_rel acquire
  ret { i64, i1 } %1
}

define { i8 addrspace(200)*, i1 } @test_cmpxchg_weak_cap(i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %exp, i8 addrspace(200)* %new) nounwind {
  %1 = cmpxchg weak i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %exp, i8 addrspace(200)* %new acq_rel acquire
  ret { i8 addrspace(200)*, i1 } %1
}

define { i32 addrspace(200)*, i1 } @test_cmpxchg_weak_cap_i32(i32 addrspace(200)* addrspace(200)* %ptr, i32 addrspace(200)* %exp, i32 addrspace(200)* %new) nounwind {
  %1 = cmpxchg weak i32 addrspace(200)* addrspace(200)* %ptr, i32 addrspace(200)* %exp, i32 addrspace(200)* %new acq_rel acquire
  ret { i32 addrspace(200)*, i1 } %1
}

; Test various success/failure memory orders

define { iCAPRANGE, i1 } @test_cmpxchg_weak_sc_sc(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new seq_cst seq_cst
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_sc_acquire(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new seq_cst acquire
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_sc_relaxed(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new seq_cst monotonic
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_acqrel_acquire(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new acq_rel acquire
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_acqrel_relaxed(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new acq_rel monotonic
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_release_acquire(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new release acquire
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_release_relaxed(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new release monotonic
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_acquire_acquire(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new acquire acquire
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_acquire_relaxed(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new acquire monotonic
  ret { iCAPRANGE, i1 } %0
}

define { iCAPRANGE, i1 } @test_cmpxchg_weak_relaxed_relaxed(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new) nounwind {
entry:
  %0 = cmpxchg weak iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %exp, iCAPRANGE %new monotonic monotonic
  ret { iCAPRANGE, i1 } %0
}
