// RUN: %clang_cc1 %s -triple cheri-unknown-freebsd -DALIGN=1 -verify
// RUN: %clang_cc1 %s -triple cheri-unknown-freebsd -fsyntax-only -ast-dump | FileCheck %s

#ifdef ALIGN
void f() {
  unsigned long foo[8];
  ((int * __capability *)foo)[0] = 0; // expected-error {{cast from 'unsigned long *' to 'int * __capability *' increases required alignment from 8 to 32}} expected-note{{use __builtin_assume_aligned(..., sizeof(void* __capability)) if you know that the source type is sufficiently aligned}}
}
#else
void g() {
  char * __capability x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char *' <CHERICapabilityToPointer>
  char *y = (__cheri_cast char *)x;
}

void h() {
  char *x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>
  char * __capability y = (__cheri_cast char * __capability)x;
}
#endif
