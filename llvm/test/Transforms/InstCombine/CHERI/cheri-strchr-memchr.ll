; RUN: %cheri_opt %s -instcombine -S | FileCheck %s
; This sandbox-ABI test case was manually constructed from non-CHERI IR (by
; replacing i8* with i8 addrspace(200)* etc. and changing the datalayout and
; triple) because the test requires that the call to strchr is a direct call
; ModuleID = 'cheri-strchr-memchr.c'
source_filename = "cheri-strchr-memchr.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@.str = private unnamed_addr addrspace(200) constant [7 x i8] c"#'0- +\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define void @get_next_format() #0 {
entry:
  %f = alloca i8 addrspace(200)*, align 32, addrspace(200)
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %1 = load i8, i8 addrspace(200)* %0, align 1
  %conv = sext i8 %1 to i32
  %tobool = icmp ne i32 %conv, 0
  br i1 %tobool, label %land.rhs, label %land.end

; CHECK: memchr.bits
land.rhs:                                         ; preds = %while.cond
  %2 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %3 = load i8, i8 addrspace(200)* %2, align 1
  %conv1 = sext i8 %3 to i32
  %call = call i8 addrspace(200)* @strchr(i8 addrspace(200)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(200)* @.str, i32 0, i32 0), i32 %conv1)
  %tobool2 = icmp ne i8 addrspace(200)* %call, null
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %4 = phi i1 [ false, %while.cond ], [ %tobool2, %land.rhs ]
  br i1 %4, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %5 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %f, align 32
  %incdec.ptr = getelementptr inbounds i8, i8 addrspace(200)* %5, i32 1
  store i8 addrspace(200)* %incdec.ptr, i8 addrspace(200)* addrspace(200)* %f, align 32
  br label %while.cond

while.end:                                        ; preds = %land.end
  ret void
}

declare i8 addrspace(200)* @strchr(i8 addrspace(200)*, i32) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (http://llvm.org/git/clang.git c3aa15f4357315da8260ac267b867257d9a49f2e) (git@github.com:RichardsonAlex/llvm-cheri.git 9503c06be3da8644e2f1d2d30adbd17bc2c18572)"}
