// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -fsyntax-only -ast-dump %s | FileCheck %s
void snprintf (char * s) { }

template<typename P >
void as_string(P sprintf_like) { }

void to_string() {
  // CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} sprintf_like 'void (* __capability)(char * __capability)':'void (* __capability)(char * __capability)'
  as_string(snprintf);
}
