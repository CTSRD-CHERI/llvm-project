; Check that we can generate sensible code for atomic operations using capability pointers on capabilities
; in both hybrid and purecap mode.
; See https://github.com/CTSRD-CHERI/llvm-project/issues/470
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=PURECAP
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=HYBRID

define { i8, i1 } @test_cmpxchg_strong_i8(ptr addrspace(200) %ptr, i8 %exp, i8 %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, i8 %exp, i8 %new acq_rel acquire
  ret { i8, i1 } %1
}

define { i16, i1 } @test_cmpxchg_strong_i16(ptr addrspace(200) %ptr, i16 %exp, i16 %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, i16 %exp, i16 %new acq_rel acquire
  ret { i16, i1 } %1
}

define { i32, i1 } @test_cmpxchg_strong_i32(ptr addrspace(200) %ptr, i32 %exp, i32 %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, i32 %exp, i32 %new acq_rel acquire
  ret { i32, i1 } %1
}

define { i64, i1 } @test_cmpxchg_strong_i64(ptr addrspace(200) %ptr, i64 %exp, i64 %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, i64 %exp, i64 %new acq_rel acquire
  ret { i64, i1 } %1
}

define { ptr addrspace(200) , i1 } @test_cmpxchg_strong_cap(ptr addrspace(200) %ptr, ptr addrspace(200) %exp, ptr addrspace(200) %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, ptr addrspace(200) %exp, ptr addrspace(200) %new acq_rel acquire
  ret { ptr addrspace(200) , i1 } %1
}

define { i8, i1 } @test_cmpxchg_weak_i8(ptr addrspace(200) %ptr, i8 %exp, i8 %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, i8 %exp, i8 %new acq_rel acquire
  ret { i8, i1 } %1
}

define { i16, i1 } @test_cmpxchg_weak_i16(ptr addrspace(200) %ptr, i16 %exp, i16 %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, i16 %exp, i16 %new acq_rel acquire
  ret { i16, i1 } %1
}

define { i32, i1 } @test_cmpxchg_weak_i32(ptr addrspace(200) %ptr, i32 %exp, i32 %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, i32 %exp, i32 %new acq_rel acquire
  ret { i32, i1 } %1
}

define { i64, i1 } @test_cmpxchg_weak_i64(ptr addrspace(200) %ptr, i64 %exp, i64 %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, i64 %exp, i64 %new acq_rel acquire
  ret { i64, i1 } %1
}

define { ptr addrspace(200) , i1 } @test_cmpxchg_weak_cap(ptr addrspace(200) %ptr, ptr addrspace(200) %exp, ptr addrspace(200) %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, ptr addrspace(200) %exp, ptr addrspace(200) %new acq_rel acquire
  ret { ptr addrspace(200) , i1 } %1
}

