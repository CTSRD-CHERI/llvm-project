; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = '/tmp/spill.c'
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: noinline nounwind optnone
define dso_local chericcallcce void @_Z9usesStackv() addrspace(200) #0 {
; CHECK-LABEL: _Z9usesStackv
; Make sure that the stack is what we expect.  If this changes, we need to
; update the number in the export table below.
; CHECK: cincoffset      csp, csp, -48
entry:
  %x = alloca [8 x i32], align 4, addrspace(200)
  %arraydecay = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %x, i32 0, i32 0
  %0 = bitcast i32 addrspace(200)* %arraydecay to i8 addrspace(200)*
  call void @foo(i8 addrspace(200)* %0)
  ret void
}

declare dso_local void @foo(i8 addrspace(200)*) addrspace(200) #1

; Function Attrs: noinline nounwind optnone
; CHECK-LABEL: _Z11usesNoStackv
; Make sure we don't allocate anything on the stack here.
; CHECK-NOT: csp
define dso_local chericcallcce void @_Z11usesNoStackv() addrspace(200) #0 {
entry:
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local chericcallcce void @_Z13usesHugeStackv() addrspace(200) #0 {
; CHECK-LABEL: _Z13usesHugeStackv
; Make sure that we allocate a load of things on the stack, it doesn't matter
; if this changes to another large value.
; CHECK: cincoffset      csp, csp, -2032
entry:
  %x = alloca [2048 x i32], align 4, addrspace(200)
  %arraydecay = getelementptr inbounds [2048 x i32], [2048 x i32] addrspace(200)* %x, i32 0, i32 0
  %0 = bitcast i32 addrspace(200)* %arraydecay to i8 addrspace(200)*
  call void @foo(i8 addrspace(200)* %0)
  ret void
}

attributes #0 = { noinline nounwind optnone "cheri-compartment"="test" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #1 = { "cheri-compartment"="test" "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 414a2537e7534b27e1916be7a3a9f1eea8f50835)"}

; Make sure that a modest stack frame is the right size (6*8 == 48)
; CHECK: __export_test__Z9usesStackv:
; CHECK-NEXT:        .half   _Z9usesStackv-__compartment_pcc_start
; CHECK-NEXT:        .byte   6

; If we don't use the stack, we should have a 0 byte here.
; CHECK: __export_test__Z11usesNoStackv:
; CHECK-NEXT:        .half   _Z11usesNoStackv-__compartment_pcc_start
; CHECK-NEXT:        .byte   0

; If we use a lot of the stack, just make sure that we use 255 here.  We should
; warn here eventually, but for now if someone writes a stack frame that needs
; more than 2040 on a microcontroller then they will have problems.
; CHECK: __export_test__Z13usesHugeStackv:
; CHECK-NEXT:        .half   _Z13usesHugeStackv-__compartment_pcc_start
; CHECK-NEXT:        .byte   255
