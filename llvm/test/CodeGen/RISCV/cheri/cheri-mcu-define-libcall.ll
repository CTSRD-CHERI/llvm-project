; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = '/usr/home/theraven/llvm-project/clang/test/CodeGen/cheri/cheri-mcu-define-libcall.c'
source_filename = "/usr/home/theraven/llvm-project/clang/test/CodeGen/cheri/cheri-mcu-define-libcall.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local cherilibcallcc i32 @add(i32 %a, i32 %b) local_unnamed_addr addrspace(200) #0 {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; Function Attrs: minsize nounwind optsize
define dso_local cherilibcallcc i32 @callFromLibcall() local_unnamed_addr addrspace(200) #1 {
entry:
; CHECK-LABEL: callFromLibcall:
; Calls to libcalls from the library compartment should be direct calls
; CHECK: ccall   foo
  %call1 = tail call cherilibcallcc i32 @foo() #3
  %add = add nsw i32 %call1, 3
  ret i32 %add
}

; Function Attrs: minsize optsize
declare cherilibcallcc i32 @foo() local_unnamed_addr addrspace(200) #2

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #1 = { minsize nounwind optsize "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #2 = { minsize optsize "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #3 = { minsize nounwind optsize }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 0}

; Both functions should be exported
; CHECK:        .section        .compartment_exports,"a",@progbits
; CHECK:        .type   __library_export_libcalls_add,@object
; CHECK:        .globl  __library_export_libcalls_add
; CHECK:        .p2align        2
; CHECK:__library_export_libcalls_add:
; CHECK:        .half   add-__compartment_pcc_start
; CHECK:        .byte   0
; CHECK:        .byte   2
; CHECK:        .size   __library_export_libcalls_add, 4
; CHECK:        .type   __library_export_libcalls_callFromLibcall,@object
; CHECK:        .globl  __library_export_libcalls_callFromLibcall
; CHECK:        .p2align        2
; CHECK:__library_export_libcalls_callFromLibcall:
; CHECK:        .half   callFromLibcall-__compartment_pcc_start
; CHECK:        .byte   0
; CHECK:        .byte   0
; CHECK:        .size   __library_export_libcalls_callFromLibcall, 4
