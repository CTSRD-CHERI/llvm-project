; RUN: %cheri128_purecap_llc %s -o - | FileCheck %s
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define i32 @foo() #0 {
entry:
  %buffer = alloca [42 x i32], align 4, addrspace(200)
  %arraydecay = getelementptr inbounds [42 x i32], [42 x i32] addrspace(200)* %buffer, i32 0, i32 0
  ; CHECK: cincoffset	$c1, $c11, 8
  ; CHECK: csetbounds	$c3, $c1, 168
  %call = call i32 @bar(i32 addrspace(200)* %arraydecay)
  ret i32 %call
}

declare i32 @bar(i32 addrspace(200)*) #1

