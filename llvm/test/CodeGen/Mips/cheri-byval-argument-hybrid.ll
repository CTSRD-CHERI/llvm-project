; This test caused assertion failures in MIPS DAG->DAG Pattern Instruction Selection
; REQUIRES: asserts
; RUN: %cheri128_llc -target-abi n64 %s -o - | FileCheck %s
; RUN: %cheri256_llc -target-abi n64 %s -o - | FileCheck %s
; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-byval-varargs.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-byval-varargs.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.arg_1000_long = type { [1000 x i64] }
%struct.arg_1000___intcap_t = type { [1000 x i8 addrspace(200)*] }

@global_1000_long_struct = common global %struct.arg_1000_long zeroinitializer, align 8
@global_1000___intcap_t_struct = common global %struct.arg_1000___intcap_t zeroinitializer, align 16

; Function Attrs: nounwind
define void @call_1000_long_byval() local_unnamed_addr #0 {
entry:
  ; tail call void @other_func(i8* bitcast (%struct.arg_1000_long* @global_1000_long_struct to i8*)) #2
  tail call void @take_1000_long_byval(%struct.arg_1000_long* byval nonnull align 8 @global_1000_long_struct) #2
  ; CHECK: jal memcpy
  ret void
}

declare void @other_func(i8*) local_unnamed_addr #1

declare void @take_1000_long_byval(%struct.arg_1000_long* byval align 8) local_unnamed_addr #1

; Function Attrs: nounwind
define void @call_1000___intcap_t_byval() local_unnamed_addr #0 {
entry:
  ; tail call void @other_func(i8* bitcast (%struct.arg_1000___intcap_t* @global_1000___intcap_t_struct to i8*)) #2
  tail call void @take_1000___intcap_t_byval(%struct.arg_1000___intcap_t* byval nonnull align 16 @global_1000___intcap_t_struct) #2
  ; CHECK: jal memcpy
  ret void
}

declare void @take_1000___intcap_t_byval(%struct.arg_1000___intcap_t* byval align 16) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 0f369215db6346fa6dda050cdef195c0faa9f4bd) (https://github.com/llvm-mirror/llvm.git 812bcb83ff25c43c3fe03d26e341eecb5ff5f544)"}
