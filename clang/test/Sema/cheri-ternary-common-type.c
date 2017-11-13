// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s
void g(const char * c) { }
void f() {
  char *p;
  const char *rshprog;
  g(p ? p+1 : rshprog);
}
// expected-no-diagnostics
// CHECK: ConditionalOperator {{.*}} {{.*}} 'const char * __capability'
