; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
;; The CToPtr instruction is not part of ISAv9, check that we emit the
;; optimized form of `x.tag ? x.addr : 0` instead.
; RUN: %riscv64_cheri_purecap_llc < %s | FileCheck %s --check-prefix=ISAV9

;; (int_cheri_cap_from_ptr x, y) -> y == 0 ? null : csetaddr(x, y)
define dso_local i64 @cap_to_ptr(i64 addrspace(200)* %dst, i8 addrspace(200)* %auth, i8 addrspace(200)* %cap) nounwind {
; ISAV9-LABEL: cap_to_ptr:
; ISAV9:       # %bb.0: # %entry
; ISAV9-NEXT:    cgettag a1, ca2
; ISAV9-NEXT:    neg a1, a1
; ISAV9-NEXT:    and a1, a2, a1
; ISAV9-NEXT:    sd a1, 0(ca0)
; ISAV9-NEXT:    mv a0, a1
; ISAV9-NEXT:    ret
entry:
  %new = call i64 @llvm.cheri.cap.to.pointer.i64(i8 addrspace(200)* %auth, i8 addrspace(200)* %cap)
  store i64 %new, i64 addrspace(200)* %dst, align 16
  ret i64 %new
}

define dso_local i64 @cap_from_ptr_ddc(i64 addrspace(200)* %dst, i8 addrspace(200)* %cap) nounwind {
; ISAV9-LABEL: cap_from_ptr_ddc:
; ISAV9:       # %bb.0: # %entry
; ISAV9-NEXT:    cgettag a2, ca1
; ISAV9-NEXT:    neg a2, a2
; ISAV9-NEXT:    and a1, a1, a2
; ISAV9-NEXT:    sd a1, 0(ca0)
; ISAV9-NEXT:    mv a0, a1
; ISAV9-NEXT:    ret
entry:
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %new = call i64 @llvm.cheri.cap.to.pointer.i64(i8 addrspace(200)* %ddc, i8 addrspace(200)* %cap)
  store i64 %new, i64 addrspace(200)* %dst, align 16
  ret i64 %new
}

declare i64 @llvm.cheri.cap.to.pointer.i64(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.ddc.get()
