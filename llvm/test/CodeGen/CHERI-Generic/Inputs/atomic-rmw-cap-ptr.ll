; Check that we can generate sensible code for atomic operations using capability pointers
; https://github.com/CTSRD-CHERI/llvm-project/issues/470
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-ATOMICS
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=PURECAP,PURECAP-LIBCALLS
@IFNOT-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=+a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-ATOMICS
@IF-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -mattr=-a < %s | FileCheck %s --check-prefixes=HYBRID,HYBRID-LIBCALLS
@IFNOT-RISCV@; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID

define iCAPRANGE @atomic_cap_ptr_xchg(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw xchg iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_add(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw add iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_sub(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw sub iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_and(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw and iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_nand(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw nand iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_or(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw or iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_xor(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw xor iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_max(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw max iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_min(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw min iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_umax(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw umax iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define iCAPRANGE @atomic_cap_ptr_umin(iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
bb:
  %tmp = atomicrmw umin iCAPRANGE addrspace(200)* %ptr, iCAPRANGE %val seq_cst
  ret iCAPRANGE %tmp
}

define float @atomic_cap_ptr_fadd(float addrspace(200)* %ptr, float %val) nounwind {
bb:
  %tmp = atomicrmw fadd float addrspace(200)* %ptr, float %val seq_cst
  ret float %tmp
}

define float @atomic_cap_ptr_fsub(float addrspace(200)* %ptr, float %val) nounwind {
bb:
  %tmp = atomicrmw fsub float addrspace(200)* %ptr, float %val seq_cst
  ret float %tmp
}
