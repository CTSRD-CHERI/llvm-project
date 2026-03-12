; See scheduling-test-n64.ll
; RUN: %cheri_purecap_llc %s -o - -disable-mips-delay-filler | FileCheck %s -check-prefix PURECAP

; PURECAP:      cld	$1, $zero, 0($c3)
; PURECAP-NEXT: cld	$2, $zero, 0($c4)
; PURECAP-NEXT: csub	$3, $c3, $c4
; PURECAP-NEXT: dsubu $1, $1, $2
; PURECAP-NEXT: daddu $2, $1, $3

; Function Attrs: noinline nounwind uwtable
define i64 @test2(ptr addrspace(200) %a1, ptr addrspace(200) %a2) addrspace(200) nounwind {
entry:
  %0 = load i64, ptr addrspace(200) %a1, align 8
  %1 = load i64, ptr addrspace(200) %a2, align 8
  %2 = call i64 @llvm.cheri.cap.diff.i64(ptr addrspace(200) %a1, ptr addrspace(200) %a2)
  %sub = sub nsw i64 %0, %1
  %add = add nsw i64 %sub, %2
  ret i64 %add
}

; Function Attrs: nounwind readnone willreturn
declare i64 @llvm.cheri.cap.diff.i64(ptr addrspace(200), ptr addrspace(200)) addrspace(200)

; PURECAP:      cld $1, $zero, 0($c3)
; PURECAP-NEXT: cld $2, $zero, 0($c4)
; PURECAP-NEXT: daddu $3, $4, $5
; PURECAP-NEXT: dsubu $1, $1, $2
; PURECAP-NEXT: daddu $2, $1, $3

; Function Attrs: noinline nounwind uwtable
define i64 @test3(ptr addrspace(200) %a1, ptr addrspace(200) %a2, i64 signext %extra1, i64 signext %extra2) addrspace(200) nounwind {
entry:
  %0 = load i64, ptr addrspace(200) %a1, align 8
  %1 = load i64, ptr addrspace(200) %a2, align 8
  %add = add nsw i64 %extra1, %extra2
  %sub = sub nsw i64 %0, %1
  %add1 = add nsw i64 %sub, %add
  ret i64 %add1
}
