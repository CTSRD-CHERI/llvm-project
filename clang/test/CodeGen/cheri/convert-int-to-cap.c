// RUN: %cheri_cc1 -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=relative | FileCheck %s --check-prefixes=AST,AST-RELATIVE,C-AST --allow-unused-prefixes
// RUN: %cheri_cc1 -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=address | FileCheck %s --check-prefixes=AST,AST-ADDRESS,C-AST --allow-unused-prefixes
// RUN: %cheri_cc1 -xc++ -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=relative | FileCheck %s --check-prefixes=AST,AST-RELATIVE,CXX-AST  --allow-unused-prefixes
// RUN: %cheri_cc1 -xc++ -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=address | FileCheck %s --check-prefixes=AST,AST-ADDRESS,CXX-AST --allow-unused-prefixes

// We pipe through opt -mem2reg to make the test checks shorter (but keep -O0 to avoid seeing any instcombine folds)
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=relative -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=relative %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,RELATIVE --allow-unused-prefixes
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=address,c-address -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=address %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,ADDRESS --allow-unused-prefixes
// RUN: %cheri_cc1 -xc++ -Wno-error=cheri-capability-misuse -verify=relative -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=relative %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,RELATIVE,CXX-CHECK --allow-unused-prefixes
// RUN: %cheri_cc1 -xc++ -Wno-error=cheri-capability-misuse -verify=address -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=address %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,ADDRESS,CXX-CHECK --allow-unused-prefixes

#ifdef __cplusplus
extern "C" {
#endif

/// Only Sema checks for the explicit mode since it won't compile.
// RUN: %cheri_cc1 -fsyntax-only %s -cheri-int-to-cap=explicit -verify=explicit,c-explicit
// RUN: %cheri_cc1 -fsyntax-only %s -cheri-int-to-cap=strict -verify=explicit,c-explicit,strict
// RUN: %cheri_cc1 -fsyntax-only -xc++ %s -cheri-int-to-cap=explicit -verify=explicit
// RUN: %cheri_cc1 -fsyntax-only -xc++ %s -cheri-int-to-cap=strict -verify=explicit,strict

// relative-no-diagnostics
// TODO: could split this into a Sema+CodeGen test, but having both here is easier

// CHECK-LABEL: define {{[^@]+}}@test_long_to_capptr
// CHECK-SAME: (i64 signext [[L:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = inttoptr i64 [[L]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
char *__capability test_long_to_capptr(long l) {
  return (char *__capability)l;
  // address-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // AST-LABEL: FunctionDecl {{.+}} test_long_to_capptr
  // AST:       CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'long' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_intcap_to_capptr
// CHECK-SAME: (i8 addrspace(200)* [[L:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret i8 addrspace(200)* [[L]]
//
char *__capability test_intcap_to_capptr(__intcap l) {
  return (char *__capability)l;
  // AST-LABEL: FunctionDecl {{.+}} test_intcap_to_capptr
  // AST:       CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
  // AST-NEXT:  ImplicitCastExpr {{.+}} '__intcap' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_capptr_default
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast i8* [[P]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
char *__capability test_ptr_to_capptr_default(char *p) {
  return (char *__capability)p;
  // explicit-error@-1{{converting non-capability type 'char *' to capability type 'char * __capability' without an explicit cast}}
  // address-warning@-2{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_capptr_default
  // AST:       CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_signed_literal_to_capptr_default
// CHECK-SAME: () #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*)
//
char *__capability test_signed_literal_to_capptr_default(void) {
  return (char *__capability)1;
  // address-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // address-note@-2{{use an explicit capability conversion or insert a cast to intcap_t if this is intended}}
  // AST-LABEL: FunctionDecl {{.+}} test_signed_literal_to_capptr_default
  // AST:       CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
}

#ifdef __cplusplus
// CXX-CHECK-LABEL: define {{[^@]+}}@test_ptr_to_capptr_reinterpret_cast
// CXX-CHECK-SAME: (i8* [[P:%.*]]) #0
// CXX-CHECK-NEXT:  entry:
// CXX-CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast i8* [[P]] to i8 addrspace(200)*
// CXX-CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
char *__capability test_ptr_to_capptr_reinterpret_cast(char *p) {
  return reinterpret_cast<char * __capability>(p);
  // explicit-error@-1{{converting non-capability type 'char *' to capability type 'char * __capability' without an explicit cast}}
  // address-warning@-2{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // CXX-AST-LABEL: FunctionDecl {{.+}} test_ptr_to_capptr_reinterpret_cast
  // CXX-AST:       CXXReinterpretCastExpr {{.+}} 'char * __capability' reinterpret_cast<char * __capability> <PointerToCHERICapability>
  // CXX-AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}
#endif

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_capptr_tocap
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast i8* [[P]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
char *__capability test_ptr_to_capptr_tocap(char *p) {
  return (__cheri_tocap char *__capability)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_capptr_tocap
  // AST:       CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_intcap_default
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint i8* [[P]] to i64
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr i8, i8 addrspace(200)* null, i64 [[TMP0]]
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP1]]
//
__intcap test_ptr_to_intcap_default(char *p) {
  return (__intcap)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_intcap_default
  // AST:       CStyleCastExpr {{.+}} '__intcap' <PointerToIntegral>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_intcap_tocap
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast i8* [[P]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
__intcap test_ptr_to_intcap_tocap(char *p) {
  return (__cheri_tocap __intcap)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_intcap_tocap
  // AST:       CStyleCastExpr {{.+}} '__intcap' <PointerToCHERICapability>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

/// Check that NULL/zero literal does not trigger errors in the strict mode
#ifdef __cplusplus
#define NULL __null
#else
#define NULL ((void *)0)
#endif

// CHECK-LABEL: define {{[^@]+}}@test_NULL()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret i8 addrspace(200)* null
char *__capability test_NULL(void) {
  return (char *__capability)NULL;
  // FIXME: this warning should be silenced
  // c-address-warning@-2{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // c-explicit-error@-3{{converting non-capability type 'void *' to capability}}
  // AST-LABEL:    FunctionDecl {{.+}} test_NULL
  // C-AST:        CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
  // C-AST-NEXT:   ParenExpr {{.+}} 'void *'
  // C-AST-NEXT:   CStyleCastExpr {{.+}} 'void *' <NullToPointer>
  // C-AST-NEXT:   IntegerLiteral {{.+}} <col:23> 'int' 0
  // CXX-AST:      CStyleCastExpr {{.+}} 'char * __capability' <NoOp>
  // CXX-AST-NEXT: ImplicitCastExpr {{.+}} <col:14> 'char * __capability' <NullToPointer> part_of_explicit_cast
  // CXX-AST-NEXT: GNUNullExpr {{.+}} <col:14> 'long'
}

// CHECK-LABEL: define {{[^@]+}}@test_zero_constant()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret i8 addrspace(200)* null
char *__capability test_zero_constant(void) {
  return (char *__capability)0;
  // FIXME: this warning should be silenced
  // c-address-warning@-2{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // c-address-note@-3{{use an explicit capability conversion}}
  // AST-LABEL:    FunctionDecl {{.+}} test_zero_constant
  // C-AST:        CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <NullToPointer>
  // CXX-AST:      CStyleCastExpr {{.+}} <col:10, col:30> 'char * __capability' <NoOp>
  // CXX-AST-NEXT: ImplicitCastExpr {{.+}} 'char * __capability' <NullToPointer> part_of_explicit_cast
  // AST-NEXT:     IntegerLiteral {{.+}} <col:30> 'int' 0
}

#ifdef __cplusplus
// CXX-CHECK-LABEL: define {{[^@]+}}@test_nullptr()
// CXX-CHECK-NEXT:  entry:
// CXX-CHECK-NEXT:    ret i8 addrspace(200)* null
char *__capability test_nullptr(void) {
  return (char *__capability)nullptr;
  // CXX-AST-LABEL: FunctionDecl {{.+}} test_nullptr
  // CXX-AST:       CStyleCastExpr {{.+}} <col:10, col:30> 'char * __capability' <NoOp>
  // CXX-AST:       ImplicitCastExpr {{.+}} <col:30> 'char * __capability' <NullToPointer> part_of_explicit_cast
  // CXX-AST-NEXT:  CXXNullPtrLiteralExpr {{.+}} <col:30> 'nullptr_t'
}
#endif

#ifdef __cplusplus
}; // extern "C"
#endif
