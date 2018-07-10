; This test caused assertion failures in MIPS DAG->DAG Pattern Instruction Selection
; REQUIRES: asserts
; XFAIL: *
; RUN: %cheri128_llc -target-abi purecap %s -o - | FileCheck %s
; RUN: %cheri256_llc -target-abi purecap %s -o - | FileCheck %s
; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-byval-varargs.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-byval-varargs.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.Dwarf_Error = type { [1024 x i32] }

@a = common addrspace(200) global %struct.Dwarf_Error zeroinitializer, align 4

; Function Attrs: nounwind
define i32 @main() local_unnamed_addr #0 {
entry:
  ; %tmp = alloca %struct.Dwarf_Error, align 8, addrspace(200)
  ; %call = tail call i32 @ptr_fn(%struct.Dwarf_Error addrspace(200)* @a) #3
  ; %0 = bitcast %struct.Dwarf_Error addrspace(200)* %tmp to i8 addrspace(200)*
  ; call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nonnull %0, i8 addrspace(200)* bitcast (%struct.Dwarf_Error addrspace(200)* @a to i8 addrspace(200)*), i64 4096, i32 4, i1 false), !tbaa.struct !1
  %call1 = call i32 @val_fn(%struct.Dwarf_Error addrspace(200)* byval nonnull align 8 @a) #3
  ret i32 0
  ; CHECK-LABEL: main
  ; Dest: stack alloca:
  ; CHECK: cincoffset      $c1, $c11, $sp
  ; CHECK: cincoffset      $c3, $c1, $zero
  ; Src: global a
  ; CHECK: csetbounds [[GLOBAL_A:\$c[0-9]+]], $c1, $1
  ; CHECK: cincoffset      $c4, [[GLOBAL_A]], $zero
  ; Size: 1024 * 4
  ; CHECK: daddiu  $4, $zero, 4096
  ; FIXME: this should not happen: it results in a cgetpccsetoffset with a R_MIPS32 relocation....
  ; CHECK-NOT: cgetpccsetoffset        $c12, val_fn
  ; CHECK: cgetpccset $c12, $c99
}

declare i32 @ptr_fn(%struct.Dwarf_Error addrspace(200)*) #1

declare i32 @val_fn(%struct.Dwarf_Error addrspace(200)* byval align 8) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 0f369215db6346fa6dda050cdef195c0faa9f4bd) (https://github.com/llvm-mirror/llvm.git 6952b345731e6ea7246b4bc5173140b7fce21719)"}
!1 = !{i64 0, i64 4096, !2}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
