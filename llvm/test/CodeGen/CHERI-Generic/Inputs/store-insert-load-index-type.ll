; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -O0 < %s | FileCheck %s

;; Test that, when DAGCombiner folds a load-insert-store to just storing the
;; element, it uses the index rather than pointer type for the offset. Without
;; this it would crash with:
;;
;;    Assertion `Int.isZero() && "Should not create non-zero capability " "constants with SelectionDAG::getConstant()"' failed.
define void @foo(ptr addrspace(200) %p) addrspace(200) {
entry:
  %0 = load <4 x i8>, ptr addrspace(200) %p, align 4
  %1 = insertelement <4 x i8> %0, i8 0, i64 0
  store <4 x i8> %1, ptr addrspace(200) %p, align 4
  ret void
}
