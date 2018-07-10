; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'llvm/tools/clang/test/CodeGen/intcap_t.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

; Just check that it compiles, doesn't crash, does produce some output.
; CHECK: c1
; Function Attrs: nounwind readnone
define i32 @c1(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = icmp slt i64 %1, %2
  %4 = zext i1 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @c2(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = icmp eq i64 %1, %2
  %4 = zext i1 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @c3(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = icmp sgt i64 %1, %2
  %4 = zext i1 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @c4(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = icmp sge i64 %1, %2
  %4 = zext i1 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @c5(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = icmp sle i64 %1, %2
  %4 = zext i1 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @ca1(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = sub nsw i64 %1, %2
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @ca2(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = add nsw i64 %2, %1
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @ca3(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = mul nsw i64 %2, %1
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @ca4(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = sdiv i64 %1, %2
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: nounwind readnone
define i32 @p1(i8* readnone %x, i8* readnone %y) #0 {
  %1 = icmp slt i8* %x, %y
  %2 = zext i1 %1 to i32
  ret i32 %2
}

; Function Attrs: nounwind readnone
define i8 addrspace(200)* @castc(i8 addrspace(200)* readnone %a) #0 {
  ret i8 addrspace(200)* %a
}

; Function Attrs: nounwind readnone
define i8* @castp(i8 addrspace(200)* %a) #0 {
  %1 = ptrtoint i8 addrspace(200)* %a to i64
  %2 = inttoptr i64 %1 to i8*
  ret i8* %2
}

; Function Attrs: nounwind readnone
define i32 @cp(i8 addrspace(200)* %x, i8 addrspace(200)* %y) #0 {
  %1 = ptrtoint i8 addrspace(200)* %x to i64
  %2 = ptrtoint i8 addrspace(200)* %y to i64
  %3 = sub i64 %1, %2
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
