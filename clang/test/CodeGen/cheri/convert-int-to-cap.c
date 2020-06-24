// RUN: %cheri_cc1 -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=relative | FileCheck %s --check-prefixes=AST,AST-RELATIVE
// RUN: %cheri_cc1 -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=address | FileCheck %s --check-prefixes=AST,AST-ADDRESS
// RUN: %cheri_cc1 -xc++ -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=relative | FileCheck %s --check-prefixes=AST,AST-RELATIVE,CXX-AST
// RUN: %cheri_cc1 -xc++ -Wno-cheri-capability-misuse -ast-dump %s -cheri-int-to-cap=address | FileCheck %s --check-prefixes=AST,AST-ADDRESS,CXX-AST

// We pipe through opt -mem2reg to make the test checks shorter (but keep -O0 to avoid seeing any instcombine folds)
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=relative -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=relative %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,RELATIVE
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=address -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=address %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,ADDRESS
// RUN: %cheri_cc1 -xc++ -Wno-error=cheri-capability-misuse -verify=relative -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=relative %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,RELATIVE,CXX-CHECK
// RUN: %cheri_cc1 -xc++ -Wno-error=cheri-capability-misuse -verify=address -emit-llvm -o - -disable-O0-optnone -cheri-int-to-cap=address %s | opt -S -mem2reg -o - | FileCheck %s --check-prefixes=CHECK,ADDRESS,CXX-CHECK

#ifdef __cplusplus
extern "C" {
#endif

/// Only Sema checks for the explicit mode since it won't compile.
// RUN: %cheri_cc1 -fsyntax-only %s -cheri-int-to-cap=explicit -verify=explicit
// RUN: %cheri_cc1 -fsyntax-only %s -cheri-int-to-cap=strict -verify=explicit,strict
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
char *__capability test_intcap_to_capptr(__intcap_t l) {
  return (char *__capability)l;
  // AST-LABEL: FunctionDecl {{.+}} test_intcap_to_capptr
  // AST:       CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
  // AST-NEXT:  ImplicitCastExpr {{.+}} '__intcap_t':'__intcap_t' <LValueToRValue> part_of_explicit_cast
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

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_capptr_via_addr_intcap
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint i8* [[P]] to i64
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr i8, i8 addrspace(200)* null, i64 [[TMP0]]
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP1]]
//
char *__capability test_ptr_to_capptr_via_addr_intcap(char *p) {
  return (char *__capability)(__cheri_addr __intcap_t)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_capptr_via_addr_intcap
  // AST:       CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
  // AST-NEXT:  CStyleCastExpr {{.+}} '__intcap_t':'__intcap_t' <PointerToIntegral>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_intcap_default
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint i8* [[P]] to i64
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr i8, i8 addrspace(200)* null, i64 [[TMP0]]
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP1]]
//
__intcap_t test_ptr_to_intcap_default(char *p) {
  return (__intcap_t)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_intcap_default
  // AST:       CStyleCastExpr {{.+}} '__intcap_t':'__intcap_t' <PointerToIntegral>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_intcap_tocap
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast i8* [[P]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
//
__intcap_t test_ptr_to_intcap_tocap(char *p) {
  return (__cheri_tocap __intcap_t)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_intcap_tocap
  // AST:       CStyleCastExpr {{.+}} '__intcap_t':'__intcap_t' <PointerToCHERICapability>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

// CHECK-LABEL: define {{[^@]+}}@test_ptr_to_intcap_addr
// CHECK-SAME: (i8* [[P:%.*]]) #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint i8* [[P]] to i64
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr i8, i8 addrspace(200)* null, i64 [[TMP0]]
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP1]]
//
__intcap_t test_ptr_to_intcap_addr(char *p) {
  return (__cheri_addr __intcap_t)p;
  // AST-LABEL: FunctionDecl {{.+}} test_ptr_to_intcap_addr
  // AST:       CStyleCastExpr {{.+}} '__intcap_t':'__intcap_t' <PointerToIntegral>
  // AST-NEXT:  ImplicitCastExpr {{.+}} 'char *' <LValueToRValue> part_of_explicit_cast
}

#ifdef __cplusplus
}; // extern "C"
#endif
