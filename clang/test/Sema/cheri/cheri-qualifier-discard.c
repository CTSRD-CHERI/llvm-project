// RUN:  %cheri_cc1 -fsyntax-only -ast-dump %s | FileCheck %s

// CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} x 'int * __capability const'
void f(int * __capability const x);
