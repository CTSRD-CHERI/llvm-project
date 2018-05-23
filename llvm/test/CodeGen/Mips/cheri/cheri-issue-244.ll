; RUN: %cheri_purecap_llc %s -o /dev/null

; ModuleID = '/local/scratch/alr48/cheri/llvm-master/tools/clang/test/CodeGen/cheri-issue-244.c'
source_filename = "/local/scratch/alr48/cheri/llvm-master/tools/clang/test/CodeGen/cheri-issue-244.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.b = type { i32 addrspace(200)* }

@e = local_unnamed_addr addrspace(200) global %struct.b { i32 addrspace(200)* addrspacecast (i32* bitcast (i32 ()* @c to i32*) to i32 addrspace(200)*) }, align 32

; Function Attrs: nounwind
define internal i32 @c() #0 {
entry:
  %call = tail call i32 (...) @dladdr(i8 addrspace(200)* inreg addrspacecast (i8* bitcast (i32 ()* @c to i8*) to i8 addrspace(200)*)) #2
  ret i32 undef
}

declare i32 @dladdr(...) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 5.0.0 "}

