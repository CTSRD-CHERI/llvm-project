// RUN: %cheri_cc1 %s -ast-dump | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -ast-dump | FileCheck %s

/// Crashed with "Invalid type passed to getNonProvenanceCarryingType" due to
/// not handling AtomicType
// CHECK:      VarDecl {{.*}} <{{.*}}:[[@LINE+4]]:1, col:22> col:18 x '_Atomic(__intcap)' cinit
// CHECK-NEXT:   ImplicitCastExpr {{.*}} <col:22> '_Atomic(__intcap __attribute__((cheri_no_provenance)))' <NonAtomicToAtomic>
// CHECK-NEXT:     ImplicitCastExpr {{.*}} <col:22> '__intcap __attribute__((cheri_no_provenance))':'__intcap' <IntegralCast>
// CHECK-NEXT:       IntegerLiteral {{.*}} <col:22> 'int' 0
_Atomic __intcap x = 0;

/// Check we preserve typedefs where possible
typedef __intcap T;
typedef _Atomic T AT;
// CHECK:      VarDecl {{.*}} <{{.*}}:[[@LINE+4]]:1, col:8> col:4 y 'AT':'_Atomic(T)' cinit
// CHECK-NEXT:   ImplicitCastExpr {{.*}} <col:8> '_Atomic(T __attribute__((cheri_no_provenance)))' <NonAtomicToAtomic>
// CHECK-NEXT:     ImplicitCastExpr {{.*}} <col:8> '__intcap __attribute__((cheri_no_provenance))':'__intcap' <IntegralCast>
// CHECK-NEXT:       IntegerLiteral {{.*}} <col:8> 'int' 0
AT y = 0;

/// Check that we don't duplicate attributes when preserving typdefs
typedef __intcap __attribute__((cheri_no_provenance)) TNP;
typedef _Atomic TNP ATNP;
// CHECK:      VarDecl {{.*}} <{{.*}}:[[@LINE+4]]:1, col:10> col:6 z 'ATNP':'_Atomic(TNP)' cinit
// CHECK-NEXT:   ImplicitCastExpr {{.*}} <col:10> 'ATNP':'_Atomic(TNP)' <NonAtomicToAtomic>
// CHECK-NEXT:     ImplicitCastExpr {{.*}} <col:10> '__intcap __attribute__((cheri_no_provenance))':'__intcap' <IntegralCast>
// CHECK-NEXT:       IntegerLiteral {{.*}} <col:10> 'int' 0
ATNP z = 0;
