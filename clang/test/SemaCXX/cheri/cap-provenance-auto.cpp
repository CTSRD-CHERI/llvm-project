/// Check that auto does not add the no_provenance attribute since we could
/// store a provenance carrying value later if it's not const.
// TODO: we could add it for const types?
// RUN: %cheri_purecap_cc1 -std=c++2a -xc++ %s -verify -fsyntax-only -ast-dump | FileCheck %s
// expected-no-diagnostics

typedef __uintcap_t uintptr_t;
#define __no_provenance __attribute__((cheri_no_provenance))
typedef __no_provenance __uintcap_t no_provenance_uintptr_t;

void test_auto(uintptr_t prov, no_provenance_uintptr_t noprov) {
  // CHECK-LABEL: {{.+}} test_auto 'void (uintptr_t, no_provenance_uintptr_t)'
  auto zero = static_cast<uintptr_t>(0); // CHECK: VarDecl {{.+}} <col:3, col:39> col:8 used zero '__uintcap_t':'__uintcap_t' cinit{{$}}
  const auto const_zero = static_cast<uintptr_t>(0); // CHECK: VarDecl {{.+}} <col:3, col:51> col:14 const_zero 'const __uintcap_t':'const __uintcap_t' cinit{{$}}
  zero = prov;
  zero = prov;
  zero = noprov;
}

void test_auto_assign(uintptr_t prov, no_provenance_uintptr_t noprov) {
  // CHECK-LABEL: {{.+}} test_auto_assign 'void (uintptr_t, no_provenance_uintptr_t)'
  auto zero = noprov; // CHECK: VarDecl {{.+}} <col:3, col:15> col:8 used zero '__uintcap_t':'__uintcap_t' cinit
  const auto const_zero = noprov; // CHECK: VarDecl {{.+}} <col:3, col:27> col:14 used const_zero 'const __uintcap_t':'const __uintcap_t' cinit
  zero = const_zero;
  zero = prov;
  zero = noprov;
}

void test_return_type_lambda(uintptr_t prov, no_provenance_uintptr_t noprov) {
  // CHECK-LABEL: {{.+}} test_return_type_lambda 'void (uintptr_t, no_provenance_uintptr_t)'
  auto lambda = []() { return static_cast<uintptr_t>(0); }; // CHECK: CXXMethodDecl {{.+}} <col:20, col:58> col:17 used constexpr operator() '__uintcap_t () const' inline
  auto from_lambda = lambda(); // CHECK: VarDecl {{.+}} <col:3, col:29> col:8 used from_lambda '__uintcap_t':'__uintcap_t' cinit
  const auto const_from_lambda = lambda(); // CHECK: VarDecl {{.+}} <col:3, col:41> col:14 referenced const_from_lambda 'const __uintcap_t':'const __uintcap_t' cinit
  from_lambda = const_from_lambda;
  from_lambda = prov;
  from_lambda = noprov;
}

auto return_noprov() { // CHECK: FunctionDecl {{.+}} <line:[[@LINE]]:1, line:[[@LINE+2]]:1> line:[[@LINE]]:6 return_noprov '__uintcap_t ()'
  return static_cast<uintptr_t>(0);
}
