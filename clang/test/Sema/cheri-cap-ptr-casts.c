// RUN: %clang_cc1 %s -triple cheri-unknown-freebsd -DALIGN=1 -verify
// RUN: %clang_cc1 %s -triple cheri-unknown-freebsd -fsyntax-only -ast-dump | FileCheck %s

#ifdef ALIGN
void f() {
  unsigned long foo[8];
  ((int * __capability *)foo)[0] = 0; // expected-error {{cast from 'unsigned long *' to 'int * __capability *' increases required alignment from 8 to 32}}
}
#else
void g() {
  __capability char *x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char *' <CHERICapabilityToPointer>
  char *y = (char *)x;
}

void h() {
  char *x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>
  __capability char *y = (__capability char *)x;
}
#endif
