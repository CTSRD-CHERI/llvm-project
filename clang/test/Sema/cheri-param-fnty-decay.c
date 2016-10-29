// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only %s -target-abi sandbox -ast-dump %s | FileCheck %s

void f(int (fn)(void)) {
  // CHECK: FunctionDecl {{.*}} {{.*}} {{.*}} f 'void (int (* __capability)(void))'
}
