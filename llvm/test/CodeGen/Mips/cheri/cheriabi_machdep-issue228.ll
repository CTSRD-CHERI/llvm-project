; RUN: llc -o /dev/null -mtriple=cheri-unknown-freebsd -relocation-model=static -mcpu=mips64 -mattr=+noabicalls -target-abi n64 -mips-ssection-threshold=0 %s 
target triple = "cheri-unknown-freebsd"
; https://github.com/CTSRD-CHERI/llvm/issues/228

; Function Attrs: inlinehint nounwind
define void @CHERIABI_SYS_mknodat_fill_uap() #0 {
cheriabi_fetch_syscall_arg.exit119:
  %0 = ptrtoint i8 addrspace(200)* undef to i16
  store i16 %0, i16* undef, align 2, !tbaa !1
  unreachable
}

define void @CHERIABI_SYS_mknodat_fill_uap1() #0 {
cheriabi_fetch_syscall_arg.exit119:
  %0 = inttoptr i16 undef to i8 addrspace(200)*
  store i8 addrspace(200)* %0, i8 addrspace(200)** undef, align 32, !tbaa !1
  unreachable
}

define void @CHERIABI_SYS_mknodat_fill_uap2(i8 addrspace(200)* %arg) #0 {
cheriabi_fetch_syscall_arg.exit119:
  %0 = ptrtoint i8 addrspace(200)* %arg to i16
  store i16 %0, i16* undef, align 2, !tbaa !1
  unreachable
}

define void @CHERIABI_SYS_mknodat_fill_uap3(i8 addrspace(200)* %arg, i16 * %ptr) #0 {
cheriabi_fetch_syscall_arg.exit119:
  %0 = ptrtoint i8 addrspace(200)* %arg to i16
  store i16 %0, i16* %ptr, align 2, !tbaa !1
  ret void
}

attributes #0 = { inlinehint nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64" "target-features"="+mips64,+noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0  (https://github.com/llvm-mirror/llvm.git c159742948721a0ba4abe3c320f8f250763b80bc)"}
!1 = !{!2, !7, i64 22}
!2 = !{!"mknodat_args", !3, i64 0, !5, i64 4, !3, i64 8, !3, i64 8, !6, i64 8, !3, i64 16, !3, i64 16, !7, i64 22, !3, i64 24, !3, i64 24, !8, i64 24, !3, i64 32}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!"int", !3, i64 0}
!6 = !{!"any pointer", !3, i64 0}
!7 = !{!"short", !3, i64 0}
!8 = !{!"long", !3, i64 0}
