// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s
class A { };

// CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} used a1 'A &'
void f(A& __capability a1) {
  // CHECK: VarDecl {{.*}} {{.*}} {{.*}} 'A &' cinit
  A& __capability a2 = a1;
}
