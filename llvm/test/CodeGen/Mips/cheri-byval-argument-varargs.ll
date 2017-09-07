; This test caused assertion failures in MIPS DAG->DAG Pattern Instruction Selection
; REQUIRES: asserts
; RUN: %cheri256_llc -target-abi purecap %s -o - | FileCheck %s
; RUN: %cheri128_llc -target-abi purecap %s -o - | FileCheck %s
; we should really be getting an error when compiling this with n64 ABI (alloca in AS 200)
; RUNTODO: not %cheri_llc -target-abi n64 < %s 2>&1 | FileCheck %s -check-prefix BAD-ABI
; BAD-ABI: error: abc
; XFAIL: *
; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-inregs-param-info.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-inregs-param-info.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.Dwarf_Error = type { [1024 x i32] }

@a = common local_unnamed_addr addrspace(200) global %struct.Dwarf_Error zeroinitializer, align 4

; Function Attrs: nounwind
define i32 @fn1() local_unnamed_addr #0 {
entry:
  %tmp = alloca %struct.Dwarf_Error, align 8, addrspace(200)
  %0 = bitcast %struct.Dwarf_Error addrspace(200)* %tmp to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nonnull %0, i8 addrspace(200)* bitcast (%struct.Dwarf_Error addrspace(200)* @a to i8 addrspace(200)*), i64 4096, i32 4, i1 false), !tbaa.struct !1
  %call = call i32 (...) @fn2(%struct.Dwarf_Error addrspace(200)* byval nonnull align 8 %tmp) #3
  ret i32 undef
}

declare i32 @fn2(...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 0c91ed96d08feda61fd68f0fe034787f01cb9fa7) (https://github.com/llvm-mirror/llvm.git 6952b345731e6ea7246b4bc5173140b7fce21719)"}
!1 = !{i64 0, i64 4096, !2}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
