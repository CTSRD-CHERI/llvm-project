; RUN: %cheri128_purecap_opt -cheri-purecap-alloca %s -o - -S | FileCheck %s
target datalayout = "E-m:m-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
declare i32 @use32(i32 addrspace(200)*) #1
declare i32 @use8(i8 addrspace(200)*) #1

define i32 @foo() nounwind {
entry:
  ; CHECK-LABEL: define i32 @foo() addrspace(200)
  ; Should have increased alignment to 256:
  ; CHECK: %buffer = alloca [32768 x i32], align 256, addrspace(200)
  %buffer = alloca [32768 x i32], align 4, addrspace(200)
  %arraydecay = getelementptr inbounds [32768 x i32], [32768 x i32] addrspace(200)* %buffer, i32 0, i32 0
  %call = call i32 @use32(i32 addrspace(200)* %arraydecay)
  ret i32 %call
}

define i32 @check4096() nounwind {
entry:
  ; CHECK-LABEL: define i32 @check4096() addrspace(200)
  ; Should have increased alignment to 8:
  ; CHECK: %buffer = alloca [4096 x i8], align 8, addrspace(200)
  %buffer = alloca [4096 x i8], align 1, addrspace(200)
  %arraydecay = getelementptr inbounds [4096 x i8], [4096 x i8] addrspace(200)* %buffer, i32 0, i32 0
  %call = call i32 @use8(i8 addrspace(200)* %arraydecay)
  ret i32 %call
}

define i32 @dynamic1() {
; CHECK-LABEL: define i32 @dynamic1() addrspace(200)
entry:
  br label %next
next:
  %buffer = alloca [65536 x i32], align 4, addrspace(200)
  ; Should have increased alignment to 512:
  ; CHECK: %buffer = alloca [65536 x i32], align 512, addrspace(200)
  %arraydecay = getelementptr inbounds [65536 x i32], [65536 x i32] addrspace(200)* %buffer, i32 0, i32 0
  %call = call i32 @use32(i32 addrspace(200)* %arraydecay)
  ret i32 %call
}

