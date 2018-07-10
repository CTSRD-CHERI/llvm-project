; RUN: %cheri_purecap_llc -o - -O2 -cheri-cap-table-abi=legacy %s | FileCheck %s
; RUN: %cheri_purecap_llc -o - -O2 -cheri-cap-table-abi=plt %s | FileCheck %s
; ModuleID = '/home/alr48/obj/build-postmerge/llvm-256-build/cheritest_libcheri-d2fe5e-bugpoint-reduce.ll-reduced-simplified.bc'
source_filename = "cheritest_libcheri-d2fe5e-bugpoint-reduce.ll-output-14b49b1.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@cheri_errno = external thread_local(initialexec) global i32, align 4

; Function Attrs: noinline nounwind optnone
define void @test_sandbox_cs_clock_gettime_default() #0 {
entry:
  store i32 0, i32 addrspace(200)* addrspacecast (i32* @cheri_errno to i32 addrspace(200)*), align 4
  unreachable

  ; CHECK:      cgetoffset	$25, $c12
  ; CHECK-NEXT: lui	$1, %hi(%neg(%gp_rel(test_sandbox_cs_clock_gettime_default)))
  ; CHECK-NEXT: daddu	$1, $1, $25
  ; CHECK-NEXT: daddiu	$1, $1, %lo(%neg(%gp_rel(test_sandbox_cs_clock_gettime_default)))
  ; CHECK:      ld	$1, %gottprel(cheri_errno)($1)

}

attributes #0 = { noinline nounwind optnone }
