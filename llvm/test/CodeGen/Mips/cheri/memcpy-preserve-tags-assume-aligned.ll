; Check that llvm.memcpy() and llvm.memmove() intrinisics with nobuiltin
; attribute are always lowered to libcalls
; RUN: %cheri128_purecap_cc1 %s -O2 -o - -S
; RUN: %cheri128_purecap_cc1 %s -O2 -o - -S | FileCheck %s

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1) #2
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1) #2


; Check that __builtin_assume_aligned does the right thing and allows us to elide the memcpy call even with "must-preserve-cheri-tags" attribute


declare void @llvm.assume(i1) addrspace(200)

define void @memcpy_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) #0 {
entry:
  %local_cap_ptr.addr = alloca i8 addrspace(200)* addrspace(200)*, align 16, addrspace(200)
  %align1.addr = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %align16 = alloca i8 addrspace(200)*, align 16, addrspace(200)
  store i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* addrspace(200)* addrspace(200)* %local_cap_ptr.addr, align 16
  store i8 addrspace(200)* %align1, i8 addrspace(200)* addrspace(200)* %align1.addr, align 16
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %align1.addr, align 16
  %ptrint = ptrtoint i8 addrspace(200)* %0 to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  store i8 addrspace(200)* %0, i8 addrspace(200)* addrspace(200)* %align16, align 16
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %align16, align 16
  %2 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)* addrspace(200)* %local_cap_ptr.addr, align 16
  %3 = bitcast i8 addrspace(200)* addrspace(200)* %2 to i8 addrspace(200)*
  ; This has "must-preserve-cheri-tags"
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %1, i8 addrspace(200)* align 16 %3, i64 16, i1 false) #3
  ret void
  ; CHECK-LABEL: memcpy_assume:
  ; CHECK:      clc $c1, $zero, 0($c3)
  ; CHECK-NEXT: cjr $c17
  ; CHECK-NEXT: csc $c1, $zero, 0($c4)

}

define void @memmove_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) #0 {
entry:
  %local_cap_ptr.addr = alloca i8 addrspace(200)* addrspace(200)*, align 16, addrspace(200)
  %align1.addr = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %align16 = alloca i8 addrspace(200)*, align 16, addrspace(200)
  store i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* addrspace(200)* addrspace(200)* %local_cap_ptr.addr, align 16
  store i8 addrspace(200)* %align1, i8 addrspace(200)* addrspace(200)* %align1.addr, align 16
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %align1.addr, align 16
  %ptrint = ptrtoint i8 addrspace(200)* %0 to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  store i8 addrspace(200)* %0, i8 addrspace(200)* addrspace(200)* %align16, align 16
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %align16, align 16
  %2 = load i8 addrspace(200)* addrspace(200)*, i8 addrspace(200)* addrspace(200)* addrspace(200)* %local_cap_ptr.addr, align 16
  %3 = bitcast i8 addrspace(200)* addrspace(200)* %2 to i8 addrspace(200)*
  ; This has "must-preserve-cheri-tags"
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %1, i8 addrspace(200)* align 16 %3, i64 16, i1 false) #3
  ret void
  ; CHECK-LABEL: memmove_assume:
  ; CHECK:      clc $c1, $zero, 0($c3)
  ; CHECK-NEXT: cjr $c17
  ; CHECK-NEXT: csc $c1, $zero, 0($c4)
}


attributes #0 = { noinline nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }
attributes #3 = { "must-preserve-cheri-tags" }
