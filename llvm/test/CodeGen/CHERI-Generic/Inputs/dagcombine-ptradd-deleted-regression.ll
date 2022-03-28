; This would previously crash DAGCombiner::visitPTRADD since the PTRADD
; corresponding to the second GEP would be collapsed to a no-op when
; reassociated and delete the synthesised PTRADD node, not just the ADD, which
; the folding code was not prepared for.
; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP

declare i32 @bar(i32 addrspace(200)*)

define internal i32 @foo(i32 addrspace(200)* %a, iCAPRANGE addrspace(200)* %b) nounwind {
entry:
  br label %loop

loop:
  %0 = getelementptr inbounds i32, i32 addrspace(200)* %a, iCAPRANGE 1
  %1 = load iCAPRANGE, iCAPRANGE addrspace(200)* %b, align 16
  %2 = mul iCAPRANGE 0, %1
  %3 = getelementptr inbounds i32, i32 addrspace(200)* %0, iCAPRANGE %2
  %4 = call i32 @bar(i32 addrspace(200)* %3)
  br label %loop
}
