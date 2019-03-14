; RUN: %cheri_llc %s -o - | FileCheck %s

declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)*, i64) nounwind readnone

define i8 addrspace(200)* @setBoundsUnknown(i8 addrspace(200)* %c, i64 %bounds) nounwind readnone {
entry:
  ; CHECK-LABEL: setBoundsUnknown
  ; CHECK:      jr $ra
  ; CHECK-NEXT: csetbounds $c3, $c3, $4
  ; CHECK: .end setBoundsUnknown
  %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %c, i64 %bounds)
  ret i8 addrspace(200)* %0
}

define i8 addrspace(200)* @setBoundsConstant1(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK-LABEL: setBoundsConstant1
  ; CHECK:      jr $ra
  ; CHECK-NEXT: csetbounds $c3, $c3, 1
  ; CHECK: .end setBoundsConstant1
  %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %c, i64 1)
  ret i8 addrspace(200)* %0
}

define i8 addrspace(200)* @setBoundsConstantMaxImmediate(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK-LABEL: setBoundsConstantMaxImmediate
  ; CHECK:      jr $ra
  ; CHECK-NEXT: csetbounds $c3, $c3, 2047
  ; CHECK: .end setBoundsConstantMaxImmediate
  %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %c, i64 2047)
  ret i8 addrspace(200)* %0
}

define i8 addrspace(200)* @setBoundsConstantTooBigForImm(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK-LABEL: setBoundsConstantTooBigForImm
  ; CHECK:      daddiu $1, $zero, 2048
  ; CHECK-NEXT: jr $ra
  ; CHECK-NEXT: csetbounds $c3, $c3, $1
  ; CHECK: .end setBoundsConstantTooBigForImm
  %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %c, i64 2048)
  ret i8 addrspace(200)* %0
}

define i8 addrspace(200)* @setBoundsNegative(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK-LABEL: setBoundsNegative
  ; CHECK:      daddiu $1, $zero, -2
  ; CHECK-NEXT: jr $ra
  ; CHECK-NEXT: csetbounds $c3, $c3, $1
  ; CHECK: .end setBoundsNegative
  %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %c, i64 -2)
  ret i8 addrspace(200)* %0
}
