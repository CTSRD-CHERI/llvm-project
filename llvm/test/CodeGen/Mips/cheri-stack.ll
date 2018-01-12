; RUN: %cheri_purecap_llc -O0 %s -o - | FileCheck --enable-var-scope %s
; ModuleID = 'cheri-stack.c'
; The code generated for CHERI256 is quite different here, XFAIL it for now
; XFAIL: cheri_is_256
source_filename = "cheri-stack.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p200i8(i64, i8 addrspace(200)* nocapture) #2

declare i32 @use_arg(i32 addrspace(200)*) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p200i8(i64, i8 addrspace(200)* nocapture) #2

; Function Attrs: norecurse nounwind readnone
define i32 @no_stack() local_unnamed_addr #0 {
entry:
; Check that a function that doesn't use the stack doesn't manipulate the stack
; pointer.  Note that at higher optimisation levels, the delay slot would be
; filled with the addiu.
; CHECK-LABEL: no_stack
; CHECK: addiu	$2, $zero, 1
; CHECK-NEXT: cjr	$c17
; CHECK-NEXT: nop
  ret i32 1
}

; Function Attrs: nounwind
define i32 @has_alloca() local_unnamed_addr #1 {
entry:
; Check that a function that allocates a buffer on the stack correctly derives
; it from the frame capability
; CHECK-LABEL: has_alloca
; CHECK: cincoffset	$c[[ALLOCREG:([0-9]+|sp)]], $c24
; CHECK: csetbounds	$c3, $c[[ALLOCREG]], 4

  %var = alloca i32, align 4, addrspace(200)
  %0 = bitcast i32 addrspace(200)* %var to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #4
  %call = call i32 @use_arg(i32 addrspace(200)* nonnull %var) #4
  call void @llvm.lifetime.end.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #4
  ret i32 %call
}

; Function Attrs: nounwind
define i32 @has_spill(i32 signext %x) local_unnamed_addr #1 {
entry:
; Check that we spill and reload relative to the correct frame capability and
; that we're loading from the same place that we spill
; Note: We're running this test at -O0, which means that we end up with a spill
; that's never reloaded.
; CHECK-LABEL: has_spill
; 
; CHECK: cincoffset	$c11, $c11, -[[FRAMESIZE:([0-9]+)]]
; CHECK: csc	$c17, $zero, [[C17OFFSET:([0-9]+|sp)]]($c11)
; $cfp <- $csp
; CHECK: cincoffset	$c24, $c11, $zero
; CHECK: cincoffset	$c[[ALLOCACAP:([0-9]+|sp)]], $c24, 28
; CHECK: csetbounds	$c{{([0-9]+|sp)}}, $c[[ALLOCACAP]], 4
; CHECK: csw	${{([0-9]+)}}, $zero, 12($c24)
; CHECK: cjalr	$c12, $c17
; CHECK: clw	${{([0-9]+)}}, $zero, 12($c24)
; CHECK: cincoffset	$c11, $c24, $zero
; CHECK: clc	$c17, $zero, [[C17OFFSET]]($c11)
; CHECK: cincoffset	$c11, $c11, [[FRAMESIZE]]

  %x.addr = alloca i32, align 4, addrspace(200)
  store i32 %x, i32 addrspace(200)* %x.addr, align 4, !tbaa !3
  %call = call i32 @use_arg(i32 addrspace(200)* nonnull %x.addr) #4
  %add = add nsw i32 %call, %x
  ret i32 %add
}

; Function Attrs: nounwind
define i32 @dynamic_alloca(i32 signext %x) local_unnamed_addr #1 {
entry:
; Check that we are able to handle dynamic allocations
; Again, because we're at -O0, we get a load of redundant copies
; CHECK-LABEL: dynamic_alloca
; CHECK: cincoffset	$c24, $c11, $zero
; CHECK: cmove	$c[[TEMPCAP:([0-9]+)]], $c11
; CHECK: cgetoffset	$[[OFFSET:([0-9]+|sp)]], $c[[TEMPCAP]]
; CHECK: dsubu	$[[OFFSET]], $[[OFFSET]], ${{([0-9]+|sp)}}
; CHECK: and	$[[OFFSET1:([0-9]+|sp)]], $[[OFFSET]], ${{([0-9]+|sp)}}
; CHECK: csetoffset	$c[[TEMPCAP1:([0-9]+)]], $c[[TEMPCAP]], $[[OFFSET1]]
; CHECK: csetbounds	$c[[TEMPCAP2:([0-9]+)]], $c[[TEMPCAP1]], ${{([0-9]+|sp)}}
; CHECK: cmove	$c11, $c[[TEMPCAP1]]
; CHECK: cincoffset	$c[[TEMPCAP3:([0-9]+)]], $c[[TEMPCAP2]], $zero
; CHECK: csetbounds	$c{{([0-9]+|sp)}}, $c[[TEMPCAP3]]
  %0 = zext i32 %x to i64
  %vla = alloca i32, i64 %0, align 4, addrspace(200)
  %call = call i32 @use_arg(i32 addrspace(200)* nonnull %vla) #4
  ret i32 %call
}


attributes #0 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
