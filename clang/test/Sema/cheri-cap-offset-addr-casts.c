// RUN: %cheri_purecap_cc1 -DAST -ast-dump %s 2>&1 | FileCheck --check-prefix=AST %s
// RUN: %cheri_purecap_cc1 -DTYPES -verify %s
//
// Various Sema tests for the __cheri_offset and __cheri_addr casts
//

#ifdef AST
void checkAST(char *c) {
  long x1 = (__cheri_offset long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToOffset>

  long x2 = (__cheri_addr long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToAddress>
}
#endif

#ifdef TYPES
void types_offset(char *c) {
  short x1 = (__cheri_offset short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the capability offset field}}
  unsigned short x2 = (__cheri_offset unsigned short)c; // expected-warning {{target type 'unsigned short' is smaller than the type 'long unsigned int' of the capability offset field}}
  long x3 = (__cheri_offset long)c;
  char *x4 = (__cheri_offset char*)c; // expected-error {{invalid target type 'char * __capability' for __cheri_offset}}
  long x5 = (__cheri_offset short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the capability offset field}}
  short x6 = (__cheri_offset long)c;
}

void types_addr(char *c) {
  short x1 = (__cheri_addr short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the address}}
  unsigned short x2 = (__cheri_addr unsigned short)c; // expected-warning {{target type 'unsigned short' is smaller than the type 'long unsigned int' of the address}}
  long x3 = (__cheri_addr long)c;
  char *x4 = (__cheri_addr char*)c; // expected-error {{invalid target type 'char * __capability' for __cheri_addr}}
  long x5 = (__cheri_addr short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the address}}
  short x6 = (__cheri_addr long)c;
}
#endif
