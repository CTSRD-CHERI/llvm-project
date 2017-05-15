; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'brooks.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

%struct.tvbuff = type { i32, i8 addrspace(200)* }

; Check that we are correctly selecting clwu for 32-bit loads.
; CHECK: clwu
; Function Attrs: nounwind
define %struct.tvbuff addrspace(200)* @tvb_new_with_subset(%struct.tvbuff addrspace(200)* nocapture readonly %backing, %struct.tvbuff addrspace(200)* %tvb) #0 {
entry:
  %real_data = getelementptr inbounds %struct.tvbuff, %struct.tvbuff addrspace(200)* %backing, i64 0, i32 1
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %real_data, align 32
  %foo = getelementptr inbounds %struct.tvbuff, %struct.tvbuff addrspace(200)* %tvb, i64 0, i32 0
  %1 = load i32, i32 addrspace(200)* %foo, align 4
  %2 = ptrtoint i8 addrspace(200)* %0 to i64
  %3 = zext i32 %1 to i64
  %add = add i64 %3, %2
  %4 = inttoptr i64 %add to i8 addrspace(200)*
  %real_data1 = getelementptr inbounds %struct.tvbuff, %struct.tvbuff addrspace(200)* %tvb, i64 0, i32 1
  store i8 addrspace(200)* %4, i8 addrspace(200)* addrspace(200)* %real_data1, align 32
  ret %struct.tvbuff addrspace(200)* %tvb
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.4 "}
