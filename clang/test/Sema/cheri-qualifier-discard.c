// RUN:  %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -ast-dump %s | FileCheck %s

// CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} x 'int * __capability const'
void f(int * __capability const x);
