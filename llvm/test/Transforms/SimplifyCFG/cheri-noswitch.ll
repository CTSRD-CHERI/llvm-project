; test that the Simplify CFG pass doesn't insert a switch statement for a
; capability (which leads to inserting a ptrtoint)
; RUN: %cheri_opt < %s -simplifycfg -S | FileCheck %s

source_filename = "nulltest2.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@.str = private unnamed_addr addrspace(200) constant [6 x i8] c"world\00", align 1
@.str.1 = private unnamed_addr addrspace(200) constant [6 x i8] c"hello\00", align 1

; Function Attrs: nounwind
define void @g() #0 {
entry:
  call void @puts(i8 addrspace(200)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(200)* @.str, i64 0, i64 0))
  ret void
}

; Function Attrs: nounwind
declare void @puts(i8 addrspace(200)* nocapture readonly) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #2

; Function Attrs: nounwind
define void @foo(i8 addrspace(200)* %x) local_unnamed_addr #0 {
entry:
; CHECK-NOT: %magicptr = ptrtoint i8 addrspace(200)* %x to i64
  %cmp = icmp eq i8 addrspace(200)* %x, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @g()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp eq i8 addrspace(200)* %x, null
  br i1 %cmp1, label %if.else, label %if.end3

if.else:                                          ; preds = %if.end
  call void @puts(i8 addrspace(200)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(200)* @.str.1, i64 0, i64 0))
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.else
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 (git@github.com:CTSRD-CHERI/clang.git 8017745d8dafe202ac654d5e05787a66c0b504d3) (git@github.com:CTSRD-CHERI/llvm.git e3e9b69224ae910aae53ea0c41e81a2f7d206df7)"}

