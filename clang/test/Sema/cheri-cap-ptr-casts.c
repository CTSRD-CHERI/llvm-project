// RUN: %cheri_cc1 %s  -DALIGN=1 -verify
// RUN: %cheri_cc1 %s  -fsyntax-only -ast-dump | FileCheck %s

#ifdef ALIGN
void f() {
  unsigned long foo[8];
  ((int * __capability *)foo)[0] = 0; // expected-error-re {{cast from 'unsigned long *' to 'int * __capability *' increases required alignment from 8 to {{16|32}}}} expected-note{{use __builtin_assume_aligned(..., sizeof(void* __capability)) if you know that the source type is sufficiently aligned}}
}
#else
void g() {
  char * __capability x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char *' <CHERICapabilityToPointer>
  char *y = (__cheri_fromcap char *)x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'void *' <CHERICapabilityToPointer>
  void *z = (__cheri_fromcap void *)x;
}

void h() {
  char *x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>
  char * __capability y = (__cheri_tocap char * __capability)x;
  // CHECK: CStyleCastExpr {{.*}} {{.*}} 'void * __capability' <PointerToCHERICapability>
  void * __capability z = (__cheri_tocap void * __capability)x;
}
#endif
