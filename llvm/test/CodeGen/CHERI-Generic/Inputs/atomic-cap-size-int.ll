;; Check that we can atomically update i128 (i64 for 32-bit systems)
;; For systems without double-width atomics (RISC-V, MIPS) we can use capability atomics
;; This is needed so we can report true for __atomic_always_lock_free(sizeof(uintptr_t), 0)
@IF-RISCV@; RUN: opt -data-layout="@PURECAP_DATALAYOUT@" @PURECAP_HARDFLOAT_ARGS@ -atomic-expand -S -mattr=+a < %s | FileCheck %s --check-prefix=PURECAP-IR
@IF-RISCV@; RUN: opt -data-layout="@HYBRID_DATALAYOUT@" @HYBRID_HARDFLOAT_ARGS@ -atomic-expand -S -mattr=+a < %s | FileCheck %s --check-prefix=HYBRID-IR
@IFNOT-RISCV@; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -atomic-expand -S < %s | FileCheck %s --check-prefix=PURECAP-IR
@IFNOT-RISCV@; RUN: opt @HYBRID_HARDFLOAT_ARGS@ -atomic-expand -S < %s | FileCheck %s --check-prefix=HYBRID-IR
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP
@IF-RISCV@; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a | FileCheck %s --check-prefixes=HYBRID,HYBRID-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | llc @HYBRID_HARDFLOAT_ARGS@ -mattr=-a | FileCheck %s --check-prefixes=HYBRID,HYBRID-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | llc @HYBRID_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=HYBRID
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=HYBRID-CAP-PTR,HYBRID-CAP-PTR-ATOMICS --allow-unused-prefixes
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=HYBRID-CAP-PTR,HYBRID-CAP-PTR-LIBCALLS --allow-unused-prefixes
@IFNOT-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID-CAP-PTR

define i@CAP_BITS@ @store(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  store atomic i@CAP_BITS@ %val, ptr addrspace(200) %ptr seq_cst, align @CAP_BYTES@
  ret i@CAP_BITS@ %val
}

define i@CAP_BITS@ @load(ptr addrspace(200) %ptr) nounwind {
  %val = load atomic i@CAP_BITS@, ptr addrspace(200) %ptr seq_cst, align @CAP_BYTES@
  ret i@CAP_BITS@ %val
}

define i@CAP_BITS@ @atomic_xchg(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw xchg ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_add(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw add ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_sub(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw sub ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_and(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw and ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_nand(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw nand ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_or(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw or ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_xor(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw xor ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_max(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw max ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_min(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw min ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_umax(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw umax ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define i@CAP_BITS@ @atomic_umin(ptr addrspace(200) %ptr, i@CAP_BITS@ %val) nounwind {
  %tmp = atomicrmw umin ptr addrspace(200) %ptr, i@CAP_BITS@ %val seq_cst
  ret i@CAP_BITS@ %tmp
}

define { i@CAP_BITS@, i1 } @cmpxchg_weak(ptr addrspace(200) %ptr, i@CAP_BITS@ %exp, i@CAP_BITS@ %new) nounwind {
  %1 = cmpxchg weak ptr addrspace(200) %ptr, i@CAP_BITS@ %exp, i@CAP_BITS@ %new acq_rel acquire
  ret { i@CAP_BITS@, i1 } %1
}

define { i@CAP_BITS@, i1 } @cmpxchg_strong(ptr addrspace(200) %ptr, i@CAP_BITS@ %exp, i@CAP_BITS@ %new) nounwind {
  %1 = cmpxchg ptr addrspace(200) %ptr, i@CAP_BITS@ %exp, i@CAP_BITS@ %new seq_cst seq_cst
  ret { i@CAP_BITS@, i1 } %1
}
