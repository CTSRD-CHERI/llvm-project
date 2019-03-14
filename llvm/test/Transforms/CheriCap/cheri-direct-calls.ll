; Checks that the cheri-direct-calls pass does not remove callees that are bitcasts
; RUN: %cheri_purecap_opt -S -cheri-direct-calls %s -o - | FileCheck %s
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: noinline nounwind optnone
define void @b() #0 {
entry:
  %call = call i32 bitcast (i32 (i32)* @a to i32 (i32 addrspace(200)*)*)(i32 addrspace(200)* null)
  ; CHECK: call i32 bitcast (i32 (i32)* @a to i32 (i32 addrspace(200)*)*)(i32 addrspace(200)* null)
  ret void
}

declare i32 @a(i32 signext) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #2

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }
