; CHERI-GENERIC-UTC: llc
; CHERI-GENERIC-UTC: mir
@IF-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a < %s --stop-after=branch-folder | FileCheck %s --check-prefixes=MIR
@IFNOT-RISCV@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s --stop-after=branch-folder --enable-tail-merge | FileCheck %s --check-prefixes=MIR
; Note: cat %s is needed so that update_mir_test_checks.py does not process these RUN lines.
@IF-RISCV@; RUN: cat %s | llc @PURECAP_HARDFLOAT_ARGS@ -mattr=+a | FileCheck %s
@IFNOT-RISCV@; RUN: cat %s | llc @PURECAP_HARDFLOAT_ARGS@ --enable-tail-merge | FileCheck %s
; REQUIRES: asserts

; The branch-folder MIR pass will merge the two blocks inside these functions but
; since the base pointer is distinct it will have two MachineMemOperands.
; The cmpxchg exact logic stored the exact flag in the MachineMemOperand and
; previously assumed there would only ever be one operand, so this test ensures
; we can handle the merged logic by adding separate pseudo instructions (which
; ensures that the branches with different comparisons can no longer be merged).

define dso_local signext i32 @merge_i32(i1 %cond1, ptr addrspace(200) %ptr, i32 %newval, i32 %cmpval) {
entry:
  br i1 %cond1, label %if.then, label %if.else

if.then:
  %ld1 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic1 = cmpxchg ptr addrspace(200) %ld1, i32 %cmpval, i32 %newval release monotonic, align 4
  br label %end

if.else:
  %ld2 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic2 = cmpxchg ptr addrspace(200) %ld2, i32 %cmpval, i32 %newval release monotonic, align 4
  br label %end

end:
  ret i32 0
}

define dso_local signext i32 @merge_ptr_addr(i1 %cond1, ptr addrspace(200) %ptr, ptr addrspace(200) %newval, ptr addrspace(200) %cmpval) {
entry:
  br i1 %cond1, label %if.then, label %if.else

if.then:
  %ld1 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic1 = cmpxchg ptr addrspace(200) %ld1, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

if.else:
  %ld2 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic2 = cmpxchg ptr addrspace(200) %ld2, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

end:
  ret i32 0
}

define dso_local signext i32 @merge_ptr_exact(i1 %cond1, ptr addrspace(200) %ptr, ptr addrspace(200) %newval, ptr addrspace(200) %cmpval) {
entry:
  br i1 %cond1, label %if.then, label %if.else

if.then:
  %ld1 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic1 = cmpxchg exact ptr addrspace(200) %ld1, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

if.else:
  %ld2 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic2 = cmpxchg exact ptr addrspace(200) %ld2, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

end:
  ret i32 0
}

define dso_local signext i32 @merge_ptr_mismatch_exact_flag(i1 %cond1, ptr addrspace(200) %ptr, ptr addrspace(200) %newval, ptr addrspace(200) %cmpval) {
entry:
  br i1 %cond1, label %if.then, label %if.else

if.then:
  %ld1 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic1 = cmpxchg exact ptr addrspace(200) %ld1, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

if.else:
  %ld2 = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  %atomic2 = cmpxchg ptr addrspace(200) %ld2, ptr addrspace(200) %cmpval, ptr addrspace(200) %newval release monotonic, align 16
  br label %end

end:
  ret i32 0
}
