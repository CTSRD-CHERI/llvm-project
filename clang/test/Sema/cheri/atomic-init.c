// RUN: %cheri_cc1 %s -ast-dump | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -ast-dump | FileCheck %s

/// Crashed with "Invalid type passed to getNonProvenanceCarryingType" due to
/// not handling AtomicType
// CHECK:      VarDecl {{.*}} <{{.*}}:[[@LINE+4]]:1, col:22> col:18 x '_Atomic(__intcap)' cinit
// CHECK-NEXT:   ImplicitCastExpr {{.*}} <col:22> '_Atomic(__intcap __attribute__((cheri_no_provenance)))' <NonAtomicToAtomic>
// CHECK-NEXT:     ImplicitCastExpr {{.*}} <col:22> '__intcap __attribute__((cheri_no_provenance))':'__intcap' <IntegralCast>
// CHECK-NEXT:       IntegerLiteral {{.*}} <col:22> 'int' 0
_Atomic __intcap x = 0;
