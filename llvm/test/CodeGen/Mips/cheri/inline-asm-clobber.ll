; RUNNOT: %cheri_purecap_cc1 -xc -emit-llvm -o - -O2 %s
; RUN: %cheri_purecap_llc -o - -O3 %s | FileCheck %s -enable-var-scope
;
; https://github.com/CTSRD-CHERI/llvm/issues/274
;
; Source code:
; int test(int intval, const char* ptrval) {
;   __asm__ ("syscall\n"
;   :
;   :
;   : "a0", "$c3");
;   char x = *((volatile char*)ptrval);
;   return (int)x + intval;
; }

; The capability clobber registers were previously off by one: -> $c3 would save $c4, etc.

define i32 @test_clobber_c2(i32 signext %intval, i8 addrspace(200)* %ptrval, i8 addrspace(200)* %ptrval2) local_unnamed_addr #0 {
entry:
  tail call void asm sideeffect "syscall\0A", "~{$4},~{$c2},~{$1}"() #1
  %0 = load volatile i8, i8 addrspace(200)* %ptrval, align 1
  %1 = load volatile i8, i8 addrspace(200)* %ptrval2, align 1
  %conv = sext i8 %0 to i32
  %conv2 = sext i8 %1 to i32
  %add = add nsw i32 %conv, %intval
  %add2 = add nsw i32 %conv2, %add
  ret i32 %add2
  ;
  ; here we should only be saving the int arg:
  ;
  ; CHECK-LABEL: test_clobber_c2:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: move	[[SAVED_INTARG:\$2]], $4
  ; CHECK-NEXT: #APP
  ; CHECK: syscall
  ; CHECK:      clb	[[LOAD1:\$[0-9]+]], $zero, 0($c3)
  ; CHECK-NEXT: clb	[[LOAD2:\$[0-9]+]], $zero, 0($c4)
  ; CHECK-NEXT: addu	$1, [[LOAD1]], [[SAVED_INTARG]]
  ; CHECK-NEXT: cjr	$c17
  ; CHECK-NEXT: addu	$2, [[LOAD2]], $1
  ; CHECK: .end test_clobber_c2
}

define i32 @test_clobber_c3(i32 signext %intval, i8 addrspace(200)* %ptrval, i8 addrspace(200)* %ptrval2) local_unnamed_addr #0 {
entry:
  tail call void asm sideeffect "syscall\0A", "~{$4},~{$c3},~{$1}"() #1
  %0 = load volatile i8, i8 addrspace(200)* %ptrval, align 1
  %1 = load volatile i8, i8 addrspace(200)* %ptrval2, align 1
  %conv = sext i8 %0 to i32
  %conv2 = sext i8 %1 to i32
  %add = add nsw i32 %conv, %intval
  %add2 = add nsw i32 %conv2, %add
  ret i32 %add2
  ;
  ; This function should save $c3 and the int arg:
  ;
  ; CHECK-LABEL: test_clobber_c3:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: cmove	[[SAVED_ARG1:\$c1]],  $c3
  ; CHECK-NEXT: move	[[SAVED_INTARG:\$2]], $4
  ; CHECK-NEXT: #APP
  ; CHECK: syscall
  ; CHECK:      clb	[[LOAD1:\$[0-9]+]], $zero, 0([[SAVED_ARG1]])
  ; CHECK-NEXT: clb	[[LOAD2:\$[0-9]+]], $zero, 0($c4)
  ; CHECK-NEXT: addu	$1, [[LOAD1]], [[SAVED_INTARG]]
  ; CHECK-NEXT: cjr	$c17
  ; CHECK-NEXT: addu	$2, [[LOAD2]], $1
  ; CHECK: .end test_clobber_c3
}

define i32 @test_clobber_c4(i32 signext %intval, i8 addrspace(200)* %ptrval, i8 addrspace(200)* %ptrval2) local_unnamed_addr #0 {
entry:
  tail call void asm sideeffect "syscall\0A", "~{$4},~{$c4},~{$1}"() #1
  %0 = load volatile i8, i8 addrspace(200)* %ptrval, align 1
  %1 = load volatile i8, i8 addrspace(200)* %ptrval2, align 1
  %conv = sext i8 %0 to i32
  %conv2 = sext i8 %1 to i32
  %add = add nsw i32 %conv, %intval
  %add2 = add nsw i32 %conv2, %add
  ret i32 %add2
  ;
  ; This function should save $c4 and the int arg:
  ;
  ; CHECK-LABEL: test_clobber_c4:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: cmove	[[SAVED_ARG2:\$c1]],  $c4
  ; CHECK-NEXT: move	[[SAVED_INTARG:\$2]], $4
  ; CHECK-NEXT: #APP
  ; CHECK: syscall
  ; CHECK:      clb	[[LOAD1:\$[0-9]+]], $zero, 0($c3)
  ; CHECK-NEXT: clb	[[LOAD2:\$[0-9]+]], $zero, 0([[SAVED_ARG2]])
  ; CHECK-NEXT: addu	$1, [[LOAD1]], [[SAVED_INTARG]]
  ; CHECK-NEXT: cjr	$c17
  ; CHECK-NEXT: addu	$2, [[LOAD2]], $1
  ; CHECK: .end test_clobber_c4
}

define i32 @test_clobber_c3_c4(i32 signext %intval, i8 addrspace(200)* %ptrval, i8 addrspace(200)* %ptrval2) local_unnamed_addr #0 {
entry:
  tail call void asm sideeffect "syscall\0A", "~{$4},~{$c3},~{$c4},~{$1}"() #1
  %0 = load volatile i8, i8 addrspace(200)* %ptrval, align 1
  %1 = load volatile i8, i8 addrspace(200)* %ptrval2, align 1
  %conv = sext i8 %0 to i32
  %conv2 = sext i8 %1 to i32
  %add = add nsw i32 %conv, %intval
  %add2 = add nsw i32 %conv2, %add
  ret i32 %add2
  ;
  ; This function should save $c3 and the int arg:
  ;
  ; CHECK-LABEL: test_clobber_c3_c4:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: cmove	[[SAVED_ARG2:\$c1]],  $c4
  ; CHECK-NEXT: cmove	[[SAVED_ARG1:\$c2]],  $c3
  ; CHECK-NEXT: move	[[SAVED_INTARG:\$2]], $4
  ; CHECK-NEXT: #APP
  ; CHECK: syscall
  ; CHECK:      clb	[[LOAD1:\$[0-9]+]], $zero, 0([[SAVED_ARG1]])
  ; CHECK-NEXT: clb	[[LOAD2:\$[0-9]+]], $zero, 0([[SAVED_ARG2]])
  ; CHECK-NEXT: addu	$1, [[LOAD1]], [[SAVED_INTARG]]
  ; CHECK-NEXT: cjr	$c17
  ; CHECK-NEXT: addu	$2, [[LOAD2]], $1
  ; CHECK: .end test_clobber_c3_c4
}
