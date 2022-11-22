; RUN: %cheri_purecap_llc %s -o - | FileCheck %s

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @imm(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: 	cincoffset	$c3, $c3, 1023
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 1023
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @imm1(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: 	cincoffset	$c3, $c3, -1024
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 -1024
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @reg(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: daddiu	$[[TMP:[0-9]+]], $zero, 1024
  ; CHECK: 	cincoffset	$c3, $c3, $[[TMP]]
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 1024
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @reg1(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: daddiu	$[[TMP:[0-9]+]], $zero, -1025
  ; CHECK: 	cincoffset	$c3, $c3, $[[TMP]]
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 -1025
  ret i8 addrspace(200)* %add.ptr
}

attributes #0 = { norecurse nounwind readnone }
!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
