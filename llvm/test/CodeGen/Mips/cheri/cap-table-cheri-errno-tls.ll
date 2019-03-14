; RUN: sed 's/;IF-CAPTABLE: //' %s | %cheri_purecap_llc -o - -O2 -cheri-cap-table-abi=plt - | FileCheck %s -check-prefix CAPTABLE
; RUN: sed 's/;IF-LEGACY: //' %s | %cheri_purecap_llc -o - -O2 -cheri-cap-table-abi=legacy - | FileCheck %s -check-prefix LEGACY
; ModuleID = '/home/alr48/obj/build-postmerge/llvm-256-build/cheritest_libcheri-d2fe5e-bugpoint-reduce.ll-reduced-simplified.bc'
source_filename = "cheritest_libcheri-d2fe5e-bugpoint-reduce.ll-output-14b49b1.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

;IF-LEGACY: @cheri_errno = external thread_local(initialexec) global i32, align 4
;IF-CAPTABLE: @cheri_errno = external thread_local(initialexec) addrspace(200) global i32, align 4

; Function Attrs: noinline nounwind optnone
define void @test_sandbox_cs_clock_gettime_default() #0 {
entry:
;IF-LEGACY:  store i32 0, i32 addrspace(200)* addrspacecast (i32* @cheri_errno to i32 addrspace(200)*), align 4
;IF-CAPTABLE:  store i32 0, i32 addrspace(200)* @cheri_errno, align 4
  unreachable

  ; LEGACY:      cgetoffset	$25, $c12
  ; LEGACY-NEXT: lui	$1, %hi(%neg(%gp_rel(test_sandbox_cs_clock_gettime_default)))
  ; LEGACY-NEXT: daddu	$1, $1, $25
  ; LEGACY-NEXT: daddiu	$1, $1, %lo(%neg(%gp_rel(test_sandbox_cs_clock_gettime_default)))
  ; LEGACY:      ld	$1, %gottprel(cheri_errno)($1)

  ; CAPTABLE:      lui	$1, %captab_tprel_hi(cheri_errno)
  ; CAPTABLE-NEXT: daddiu	$1, $1, %captab_tprel_lo(cheri_errno)
  ; CAPTABLE-NEXT: cld	$1, $1, 0($c26)
  ; CAPTABLE-NEXT: creadhwr	$c1, $chwr_userlocal
  ; CAPTABLE-NEXT: addiu	$2, $zero, 0
  ; CAPTABLE-NEXT: csw	$zero, $1, 0($c1)

}

attributes #0 = { noinline nounwind optnone }
