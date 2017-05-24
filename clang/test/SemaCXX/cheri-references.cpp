// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -fsyntax-only -ast-dump %s | FileCheck %s
class A { };

// CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} used a1 'class A & __capability'
void f(A& __capability a1) {
  // CHECK: VarDecl {{.*}} {{.*}} {{.*}} 'class A & __capability' cinit
  A& __capability a2 = a1;
}
