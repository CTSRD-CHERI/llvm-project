; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
source_filename = "/usr/home/theraven/llvm-project/clang/test/CodeGen/cheri/cheri-mcu-call-libcall.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize nounwind optsize
define dso_local i32 @callFromNotLibcall() local_unnamed_addr addrspace(200) #0 {
entry:
; Check that these are direct calls via the import table, not going via the
; compartment switcher.
; CHECK: auipcc  ca0, %cheri_compartment_pccrel_hi(__library_import_libcalls_add)
; CHECK: cincoffset      ca0, ca0, %cheri_compartment_pccrel_lo(.LBB0_1)
; CHECK: clc     ca2, 0(ca0)
; CHECK: addi    a0, zero, 1
; CHECK: addi    a1, zero, 2
; CHECK: cjalr   ca2
  %call = tail call cherilibcallcc i32 @add(i32 1, i32 2) #2
; CHECK: auipcc  ca0, %cheri_compartment_pccrel_hi(__library_import_libcalls_foo)
; CHECK: cincoffset      ca0, ca0, %cheri_compartment_pccrel_lo(.LBB0_2)
; CHECK: clc     ca0, 0(ca0)
; CHECK: cjalr   ca0
  %call1 = tail call cherilibcallcc i32 @foo() #2
  %add = add nsw i32 %call1, %call
  ret i32 %add
}

; Function Attrs: minsize optsize
declare cherilibcallcc i32 @add(i32, i32) local_unnamed_addr addrspace(200) #1

; Function Attrs: minsize optsize
declare cherilibcallcc i32 @foo() local_unnamed_addr addrspace(200) #1

attributes #0 = { minsize nounwind optsize "cheri-compartment"="foo" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #1 = { minsize optsize "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #2 = { minsize nounwind optsize }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 0}

; Check that the low bit is set in the import table entries.
; CHECK: __library_import_libcalls_add:
; CHECK:         .word   __library_export_libcalls_add+1
; CHECK:         .word   0
; CHECK:         .size   __library_import_libcalls_add, 8
; CHECK: __library_import_libcalls_foo:
; CHECK:         .word   __library_export_libcalls_foo+1
