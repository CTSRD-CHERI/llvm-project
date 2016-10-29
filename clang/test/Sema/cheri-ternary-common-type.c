// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -fsyntax-only -ast-dump %s | FileCheck %s
void g(const char * c) { }
void f() {
  char *p;
  const char *rshprog;
  g(p ? p+1 : rshprog);
}
// expected-no-diagnostics
// CHECK: ConditionalOperator {{.*}} {{.*}} 'const char * __capability'
