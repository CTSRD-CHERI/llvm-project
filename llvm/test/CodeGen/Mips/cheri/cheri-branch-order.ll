; RUN: %cheri_llc -relocation-model=pic %s -o - -O0 | FileCheck %s

; Check that the store of the global pointer is not moved after the branch and
; that the branch is replaced by a branch-on-tag-set.
; CHECK: cbts	$c{{[0-9]+}}, .LBB0

@.str = private unnamed_addr constant [20 x i8] c"Storing in slot %p\0A\00", align 1

; Function Attrs: nounwind
define ptr @x(ptr addrspace(200) %y) #0 {
entry:
  %retval = alloca ptr, align 8
  %y.addr = alloca ptr addrspace(200), align 32
  %slot = alloca ptr, align 8
  store ptr addrspace(200) %y, ptr %y.addr, align 32
  %0 = load ptr addrspace(200), ptr %y.addr, align 32
  %1 = bitcast ptr addrspace(200) %0 to ptr addrspace(200)
  %2 = call i1 @llvm.cheri.cap.tag.get(ptr addrspace(200) %1)
  br i1 %2, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store ptr null, ptr %retval, align 8
  br label %return

if.end:                                           ; preds = %entry
  %3 = load ptr addrspace(200), ptr %y.addr, align 32
  %4 = addrspacecast ptr addrspace(200) %3 to ptr
  store ptr %4, ptr %slot, align 8
  %5 = load ptr, ptr %slot, align 8
  %call = call i32 (ptr, ...) @printf(ptr @.str, ptr %5)
  %6 = load ptr, ptr %slot, align 8
  %7 = bitcast ptr %6 to ptr
  store ptr %7, ptr %retval, align 8
  br label %return

return:                                           ; preds = %if.end, %if.then
  %8 = load ptr, ptr %retval, align 8
  ret ptr %8
}

; Function Attrs: nounwind willreturn memory(none)
declare i1 @llvm.cheri.cap.tag.get(ptr addrspace(200)) #1

declare i32 @printf(ptr, ...) #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind willreturn memory(none) }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.5 "}
