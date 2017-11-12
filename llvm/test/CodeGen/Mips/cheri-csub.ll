; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'ptrsub.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind readnone
define i64 @subp(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: csub	$2, $c3, $c4
  %0 = tail call i64 @llvm.cheri.cap.diff(i8 addrspace(200)* %a, i8 addrspace(200)* %b)
  ret i64 %0
}

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.diff(i8 addrspace(200)*, i8 addrspace(200)*) #1

attributes #0 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang cfd4f4ecf2f160cccd4fa81fa37d018449e7d38e) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm a35715647fc87fdb529c6e930c934a21c304d210)"}

