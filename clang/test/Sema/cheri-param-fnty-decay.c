// RUN: %cheri_cc1 -fsyntax-only %s -target-abi purecap -ast-dump %s | FileCheck %s

void f(int (fn)(void)) {
  // CHECK: FunctionDecl {{.*}} {{.*}} {{.*}} f 'void (int (* __capability)(void))'
}
