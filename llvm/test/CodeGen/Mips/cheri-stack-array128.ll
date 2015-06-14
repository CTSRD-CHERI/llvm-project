; RUN: llc %s -mtriple=cheri-unknown-freebsd -target-abi sandbox -mattr=+cheri128 -o - | FileCheck %s
target datalayout = "E-m:m-p200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define i32 @foo() #0 {
entry:
  %buffer = alloca [42 x i32], align 4
  %arraydecay = getelementptr inbounds [42 x i32], [42 x i32] addrspace(200)* %buffer, i32 0, i32 0
  ; CHECK: daddiu	$1, $sp, 8
  ; CHECK: csetoffset	$c1, $c11, $1
  ; CHECK: daddiu	$1, $zero, 168
  ; CHECK: csetbounds	$c3, $c1, $1
  %call = call i32 @bar(i32 addrspace(200)* %arraydecay)
  ret i32 %call
}

declare i32 @bar(i32 addrspace(200)*) #1

