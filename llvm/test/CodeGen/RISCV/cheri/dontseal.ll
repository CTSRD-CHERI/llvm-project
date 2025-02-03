; RUN: %riscv64_cheri_purecap_llc -verify-machineinstrs < %s \
; RUN:  | FileCheck %s
; RUN: %riscv64_bakewell_purecap_llc -verify-machineinstrs < %s \
; RUN:  | FileCheck %s

; Function Attrs: dont_seal noinline nounwind optnone
define dso_local void @withoutseal() addrspace(200) #0 {
; CHECK: .dont_seal withoutseal
entry:
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @withseal() addrspace(200) #1 {
; CHECK-NOT: .dont_seal withseal
entry:
  ret void
}

attributes #0 = { dont_seal noinline nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+64bit,+cap-mode,+xcheri" }
attributes #1 = { noinline nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+64bit,+cap-mode,+xcheri" }

