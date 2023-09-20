; RUN: split-file %s %t
; RUN: not llvm-as < %t/not-ptr.ll 2>&1 | FileCheck %s --check-prefix=ERR-NOT-PTR
; RUN: not llvm-as < %t/not-cap.ll 2>&1 | FileCheck %s --check-prefix=ERR-NOT-CAP
; RUN: not llvm-as < %t/wrong-order.ll 2>&1 | FileCheck %s --check-prefix=ERR-WRONG-ORDER
; RUN: llvm-as < %t/valid.ll | llvm-dis -o - | FileCheck %s --check-prefix=ROUNDTRIP

;--- wrong-order.ll
define void @f(ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c, ptr %d) {
  ; ERR-WRONG-ORDER: <stdin>:4:31: error: expected type
  ;; The "exact" flag must come after the optional "weak" flag.
  %x = cmpxchg volatile exact weak ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 16
  ret void
}

;--- not-ptr.ll
define void @f(i32* %a, i32 %b, i32 %c) {
  ; ERR-NOT-PTR: assembly parsed, but does not verify as correct!
  ; ERR-NOT-PTR-NEXT: exact flag on cmpxchg is only valid for capability types
  %x = cmpxchg exact i32* %a, i32 %b, i32 %c seq_cst seq_cst
  ret void
}

;--- not-cap.ll
target datalayout = "em:e-pf200:128:128:128:64"
define void @f(ptr addrspace(200) %a, ptr addrspace(1) %b, ptr addrspace(1) %c) {
  ; ERR-NOT-CAP: assembly parsed, but does not verify as correct!
  ; ERR-NOT-CAP-NEXT: exact flag on cmpxchg is only valid for capability types
  %x = cmpxchg exact ptr addrspace(200) %a, ptr addrspace(1) %b, ptr addrspace(1) %c seq_cst seq_cst, align 16
  ret void
}

;--- valid.ll
target datalayout = "em:e-pf200:128:128:128:64"
define void @f(ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c, ptr %d) {
  ; ROUNDTRIP: %x = cmpxchg weak exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 16
  %x = cmpxchg weak exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst
  ; ROUNDTRIP: %y = cmpxchg exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 64
  %y = cmpxchg exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 64
  ; ROUNDTRIP: %z = cmpxchg ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 16
  %z = cmpxchg ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst
  ; ROUNDTRIP: %no_align = cmpxchg exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 16
  %no_align = cmpxchg exact ptr addrspace(200) %a, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst
  ; ROUNDTRIP: %noncap_ptr = cmpxchg exact ptr %d, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 32
  %noncap_ptr = cmpxchg exact ptr %d, ptr addrspace(200) %b, ptr addrspace(200) %c seq_cst seq_cst, align 32
  ret void
}
