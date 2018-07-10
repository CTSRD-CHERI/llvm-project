; RUN: opt < %s -instcombine -S | FileCheck %s
source_filename = "copyptr.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.name_t = type { i8 addrspace(200)* }

@x = common local_unnamed_addr addrspace(200) global %struct.name_t zeroinitializer, align 16

; Function Attrs: nounwind
define void @test(%struct.name_t addrspace(200)* %str) local_unnamed_addr #0 {
entry:
  %0 = bitcast %struct.name_t addrspace(200)* %str to i8 addrspace(200)*
; Check that the memcpy is expanded to a load and store pair
; CHECK:   load i8 addrspace(200)*
; CHECK:  store i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast (%struct.name_t addrspace(200)* @x to i8 addrspace(200)*), i64 16, i32 16, i1 false), !tbaa.struct !3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang 04facbacd4cd9150b4e29d8583e9bb05c7ffdef9) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm d6c7843f07a9697edc46d95b39a5d9249cfbe0d7)"}
!3 = !{i64 0, i64 16, !4}
!4 = !{!5, !5, i64 0}
!5 = !{!"any pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
