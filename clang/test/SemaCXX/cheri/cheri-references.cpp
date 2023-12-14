// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s
class A { };

// CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} used a1 'A & __capability':'A &'
void f(A& __capability a1) {
  // CHECK: VarDecl {{.*}} {{.*}} {{.*}} 'A & __capability':'A &' cinit
  A& __capability a2 = a1;
}
