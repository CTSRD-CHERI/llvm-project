;; After merging to LLVM 15 the stack bounds pass the switch to opqaue pointers caused
;; miscompilations in the stack bounding pass (the unbounded value was used instead of
;; the bounded one due to the removal of the bitcast instructions).
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -cheri-bound-allocas -o - -S %s -debug-only=cheri-bound-allocas 2>%t.dbg| FileCheck %s
; RUN: FileCheck %s -input-file=%t.dbg -check-prefix DBG
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s -check-prefix ASM
target datalayout = "@PURECAP_DATALAYOUT@"

; DBG-LABEL: Checking function lazy_bind_args
; DBG-NEXT: cheri-bound-allocas:  -Checking if load/store needs bounds (GEP offset is 0):   %0 = load ptr addrspace(200), ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:   -Load/store size=@CAP_BYTES@, alloca size=@CAP_BYTES@, current GEP offset=0 for ptr addrspace(200)
; DBG-NEXT: cheri-bound-allocas:   -Load/store is in bounds -> can reuse $csp for   %0 = load ptr addrspace(200), ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:  -Adding stack bounds since it is passed to call:   %call = call ptr addrspace(200) @cheribsdtest_dynamic_identity_cap(ptr addrspace(200) noundef nonnull %cap)
; DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %call = call ptr addrspace(200) @cheribsdtest_dynamic_identity_cap(ptr addrspace(200) noundef nonnull %cap)
; DBG-NEXT: cheri-bound-allocas:  -Checking if load/store needs bounds (GEP offset is 0):   store ptr addrspace(200) %cap, ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:   -Load/store size=@CAP_BYTES@, alloca size=@CAP_BYTES@, current GEP offset=0 for ptr addrspace(200)
; DBG-NEXT: cheri-bound-allocas:   -Load/store is in bounds -> can reuse $csp for   store ptr addrspace(200) %cap, ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:  -Checking if load/store needs bounds (GEP offset is 0):   store ptr addrspace(200) %cap, ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:   -Stack slot used as value and not pointer -> must set bounds
; DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds: store ptr addrspace(200) %cap, ptr addrspace(200) %cap, align @CAP_BYTES@
; DBG-NEXT: cheri-bound-allocas:  -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.start.p200(i64 @CAP_BYTES@, ptr addrspace(200) nonnull %cap)
; DBG-NEXT: cheri-bound-allocas: lazy_bind_args: 2 of 5 users need bounds for   %cap = alloca ptr addrspace(200), align @CAP_BYTES@, addrspace(200)
; DBG-NEXT: lazy_bind_args: setting bounds on stack alloca to @CAP_BYTES@  %cap = alloca ptr addrspace(200), align @CAP_BYTES@, addrspace(200)

declare void @llvm.lifetime.start.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200)

declare ptr addrspace(200) @cheribsdtest_dynamic_identity_cap(ptr addrspace(200) noundef) addrspace(200)

declare void @cheribsdtest_check_cap_eq(ptr addrspace(200) noundef, ptr addrspace(200) noundef) addrspace(200)

define dso_local void @lazy_bind_args() addrspace(200) nounwind {
entry:
  %cap = alloca ptr addrspace(200), align @CAP_BYTES@, addrspace(200)
  call void @llvm.lifetime.start.p200(i64 @CAP_BYTES@, ptr addrspace(200) nonnull %cap)
  store ptr addrspace(200) %cap, ptr addrspace(200) %cap, align @CAP_BYTES@
  %call = call ptr addrspace(200) @cheribsdtest_dynamic_identity_cap(ptr addrspace(200) noundef nonnull %cap)
  %0 = load ptr addrspace(200), ptr addrspace(200) %cap, align @CAP_BYTES@
  call void @cheribsdtest_check_cap_eq(ptr addrspace(200) noundef %0, ptr addrspace(200) noundef %call)
  ret void
}
