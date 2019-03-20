// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s
void snprintf (char * s) { }

template<typename P >
void as_string(P sprintf_like) { }

void to_string() {
  // CHECK: ParmVarDecl {{.*}} {{.*}} {{.*}} sprintf_like 'void (* __capability)(char * __capability)':'void (* __capability)(char * __capability)'
  as_string(snprintf);
}
