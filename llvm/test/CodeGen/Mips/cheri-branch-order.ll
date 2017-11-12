; RUN: %cheri_llc -relocation-model=pic %s -o - -O0 | FileCheck %s
; ModuleID = 'brazdil.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Check that the store of the global pointer is not moved after the branch and
; that the branch is replaced by a branch-on-tag-set.
; CHECK: cbts	$c3, .LBB0

@.str = private unnamed_addr constant [20 x i8] c"Storing in slot %p\0A\00", align 1

; Function Attrs: nounwind
define i8* @x(i8 addrspace(200)* addrspace(200)* %y) #0 {
entry:
  %retval = alloca i8*, align 8
  %y.addr = alloca i8 addrspace(200)* addrspace(200)*, align 32
  %slot = alloca i8 addrspace(200)**, align 8
  store i8 addrspace(200)* addrspace(200)* %y, i8 addrspace(200)* addrspace(200)** %y.addr, align 32
  %0 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)** %y.addr, align 32
  %1 = bitcast i8 addrspace(200)* addrspace(200)* %0 to i8 addrspace(200)*
  %2 = call i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %1)
  %tobool = icmp ne i64 %2, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store i8* null, i8** %retval
  br label %return

if.end:                                           ; preds = %entry
  %3 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)** %y.addr, align 32
  %4 = addrspacecast i8 addrspace(200)* addrspace(200)* %3 to i8 addrspace(200)**
  store i8 addrspace(200)** %4, i8 addrspace(200)*** %slot, align 8
  %5 = load i8 addrspace(200)**, i8 addrspace(200)*** %slot, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), i8 addrspace(200)** %5)
  %6 = load i8 addrspace(200)**, i8 addrspace(200)*** %slot, align 8
  %7 = bitcast i8 addrspace(200)** %6 to i8*
  store i8* %7, i8** %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %8 = load i8*, i8** %retval
  ret i8* %8
}

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)*) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.5 "}
