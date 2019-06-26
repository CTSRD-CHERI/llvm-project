; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - -O1 | %cheri_FileCheck %s
; ModuleID = 'va.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@va_cpy = common addrspace(200) global i8 addrspace(200)* null, align 32

; Function Attrs: nounwind
; CHECK-LABEL: cpy:
; Check that locally creating a va_list and then storing it to a global works
; (Yes, this is an odd thing to do.  See libxo for a real-world example)
define void @cpy(i8 addrspace(200)* nocapture readnone %y, i32 signext %x, ...) #0 {
entry:
  %v = alloca i8 addrspace(200)*, align 32, addrspace(200)
  %0 = bitcast i8 addrspace(200)* addrspace(200)* %v to i8 addrspace(200)*
  ; Load the address of va_cpy
  ; CHECK: cmove [[VA_LIST:\$c[0-9]+]],  $c13
  ; CHECK: clcbi   $c[[CPYADDR:[0-9]+]], %captab20(va_cpy)($c2)
  ; Store the va_list (passed in $c13) in the global
  ; CHECK: cgetnull $c13
  ; CHECK: csc	[[VA_LIST]], $zero, 0($c[[CPYADDR]])
  %1 = addrspacecast i8 addrspace(200)* %0 to i8*
  call void @llvm.lifetime.start.p200i8(i64 32, i8 addrspace(200)* %0) #1
  call void @llvm.va_start.p200i8(i8 addrspace(200)* %0)
  call void @llvm.va_copy.p200i8.p200i8(i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @va_cpy to i8 addrspace(200)*), i8 addrspace(200)* %0)
  call void @llvm.va_end.p200i8(i8 addrspace(200)* %0)
  call void @llvm.lifetime.end.p200i8(i64 32, i8 addrspace(200)* %0) #1
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start.p200i8(i64, i8 addrspace(200)* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.va_start.p200i8(i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.va_copy.p200i8.p200i8(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.va_end.p200i8(i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end.p200i8(i64, i8 addrspace(200)* nocapture) #1

; Function Attrs: nounwind
; CHECK-LABEL: cpy1:
define i8 addrspace(200)* @cpy1() #0 {
entry:
  ; Check that va_copy can copy from a global to a register
  %v = alloca i8 addrspace(200)*, align 32, addrspace(200)
  ; Load the address of the global
  ; CHECK: clcbi [[CPYADDR:\$c[0-9]+]], %captab20(va_cpy)($c1)
  ; CHECK-NEXT: clc $c1, $zero, 0([[CPYADDR]])
  ; TODO: there is no need to set the bounds on the va_copy() destination
  ; since it is just a single capability store.
  ; CHECK-NEXT: csetbounds [[BOUNDED_STACK:\$c[0-9]+]], [[STACK:\$c(11|24)]], [[#CAP_SIZE]]
  ; CHECK-NEXT: csc $c1, $zero, 0([[BOUNDED_STACK]])
  ; Load the va_list into the return capability: note: 0($c11) == $c2
  ; CHECK-NEXT: clc $c3, $zero, 0([[STACK]])
  %0 = bitcast i8 addrspace(200)* addrspace(200)* %v to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 32, i8 addrspace(200)* %0) #1
  call void @llvm.va_copy.p200i8.p200i8(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @va_cpy to i8 addrspace(200)*))
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %v, align 32, !tbaa !1
  call void @llvm.lifetime.end.p200i8(i64 32, i8 addrspace(200)* %0) #1
  ret i8 addrspace(200)* %1
}

; Function Attrs: nounwind
; CHECK-LABEL: f:
define void @f(i8 addrspace(200)* nocapture readnone %y, i32 signext %x, ...) #0 {
entry:
  ; When calling from a variadic function to one that takes a va_list, we
  ; should simply move the va capability from $c13 to the relevant argument
  ; register.
  ; TODO: this could be a simple cmove!
  ; CHECK: csetbounds [[BOUNDED_STACK:\$c[0-9]+]], [[STACK:\$c(11|24)]], [[#CAP_SIZE]]
  ; CHECK-NEXT: csc $c13, $zero, 0([[BOUNDED_STACK]])
  ; CHECK-NEXT: clc $c3, $zero, 0([[STACK]])
  ; CHECK: clcbi   $c12, %capcall20(g)($c1)
  ; Call the non-variadic function and clear $c13 in the delay slot:
  ; CHECK: cjalr $c12, $c17
  ; CHECK-NEXT: cgetnull $c13

  %v = alloca i8 addrspace(200)*, align 32, addrspace(200)
  %0 = bitcast i8 addrspace(200)* addrspace(200)* %v to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 32, i8 addrspace(200)* %0) #1
  call void @llvm.va_start.p200i8(i8 addrspace(200)* %0)
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %v, align 32, !tbaa !1
  %call = call i32 @g(i32 signext 1, i32 signext 2, i8 addrspace(200)* %1) #1
  call void @llvm.va_end.p200i8(i8 addrspace(200)* %0)
  call void @llvm.lifetime.end.p200i8(i64 32, i8 addrspace(200)* %0) #1
  ret void
}

declare i32 @g(i32 signext, i32 signext, i8 addrspace(200)*) #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.pcc.get() #3

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.cap.offset.set(i8 addrspace(200)*, i64) #3

; Function Attrs: nounwind
; CHECK-LABEL: k:
define void @k(i32 addrspace(200)* %x, i32 addrspace(200)* %y) #0 {
; When calling a variadic function, we should set $c13 to the size of the arguments
entry:
; CHECK: csetbounds	$c2, $c11, [[#CAP_SIZE]]
; CHECK: ori	$[[TMP:[0-9]+]], $zero, 65495
; CHECK: candperm	$c13, $c2, $[[TMP]]
  %x.addr = alloca i32 addrspace(200)*, align 32, addrspace(200)
  %y.addr = alloca i32 addrspace(200)*, align 32, addrspace(200)
  store i32 addrspace(200)* %x, i32 addrspace(200)* addrspace(200)* %x.addr, align 32, !tbaa !1
  store i32 addrspace(200)* %y, i32 addrspace(200)* addrspace(200)* %y.addr, align 32, !tbaa !1
  %0 = bitcast i32 addrspace(200)* addrspace(200)* %x.addr to i8 addrspace(200)*
  call void (i8 addrspace(200)*, ...) @h(i8 addrspace(200)* %0, i32 addrspace(200)* addrspace(200)* nonnull %y.addr) #1
  ret void
}

declare void @h(i8 addrspace(200)*, ...) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang e07e76cb456a0033f6fed786006a3765d2e52d4b) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm 8e07a1b9f721b905c3462b537ff68fce6c5dbe53)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
