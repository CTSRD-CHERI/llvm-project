// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -fsyntax-only -ast-dump %s | FileCheck %s

class A { };

void f() {
  // CHECK: CXXNewExpr {{.*}} {{.*}} 'class A * __capability' Function
  A *a = new A;
}
