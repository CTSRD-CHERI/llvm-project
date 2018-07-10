; REQUIRES: clang
; RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -O2 -S -o - %s | FileCheck %s -check-prefixes=CHECK,LEGACY
; RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -O2 -S -o - %s | FileCheck %s -check-prefixes=CHECK,NEW
; ModuleID = 'ocsp_cl.i'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.ASN1_ITEM_st = type opaque

@a = external addrspace(200) global %struct.ASN1_ITEM_st, align 1

; Function Attrs: nounwind
; CHECK-LABEL: fn1:
; Check that clang doesn't try to get the size of struct.ASN1_ITEM_st, since
; it's not known and does not need to be
define void @fn1() #0 {
entry:
  ; Load the address of a
  ; LEGACY: ld	$[[AADDR:[0-9]+]], %got_disp(a)($gp)
  ; LEGACY: cfromddc $c1, $[[AADDR]]
  ; LEGACY: ld	$[[FN2ADDR:([0-9]+|sp)]], %call16(fn2)($gp)
  ; Call fn2
  ; LEGACY: cgetpccsetoffset	$c12, $[[FN2ADDR]]
  ; NEW: clcbi $c3, %captab20(a)($c26)
  ; CHECK: cjalr	$c12, $c17
  tail call void @fn2(%struct.ASN1_ITEM_st addrspace(200)* @a) #2
  ret void
}

declare void @fn2(%struct.ASN1_ITEM_st addrspace(200)*) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (git@github.com:CTSRD-CHERI/clang) (git@github.com:CTSRD-CHERI/llvm 3d829d6649fd95c68a8460232114f17efafc02bd)"}
