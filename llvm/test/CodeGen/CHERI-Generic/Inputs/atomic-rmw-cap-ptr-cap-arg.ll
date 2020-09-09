; Check that we can generate sensible code for atomic operations using capability pointers on capabilities
; in both hybrid and purecap mode. For RISC-V this means expanding the instruction using the explicit
; addressing mode LR/SC instructions.
; See https://github.com/CTSRD-CHERI/llvm-project/issues/470
@IF-RISCV@; RUN: %generic_cheri_purecap_llc %s -o - -mattr=+f,+a -verify-machineinstrs | FileCheck %s --check-prefixes=PURECAP,PURECAP-ATOMICS
@IF-RISCV@; RUN: %generic_cheri_purecap_llc %s -o - -mattr=+f,-a -verify-machineinstrs | FileCheck %s --check-prefixes=PURECAP,PURECAP-LIBCALLS
@IFNOT-RISCV@; RUN: %generic_cheri_purecap_llc %s -o - | FileCheck %s --check-prefix=PURECAP
@IF-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - -mattr=+f,+a -verify-machineinstrs | FileCheck %s --check-prefixes=HYBRID,HYBRID-ATOMICS
@IF-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - -mattr=+f,-a -verify-machineinstrs | FileCheck %s --check-prefixes=HYBRID,HYBRID-LIBCALLS
@IFNOT-RISCV@; RUN: %generic_cheri_hybrid_llc %s -o - | FileCheck %s --check-prefix=HYBRID

define dso_local void @atomic_cap_ptr_xchg(i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %val) nounwind {
bb:
  %tmp = atomicrmw xchg i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %val seq_cst
  ret void
}

; TODO: support all these:
; define dso_local void @atomic_cap_ptr_add(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw add i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_sub(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw sub i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_and(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw and i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_nand(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw nand i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_or(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw or i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_xor(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw xor i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_max(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw max i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_min(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw min i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_umax(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw umax i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
;
; define dso_local void @atomic_cap_ptr_umin(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val) nounwind {
; bb:
;   %tmp = atomicrmw umin i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %val seq_cst
;   ret void
; }
