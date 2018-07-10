// RUN: %cheri_cc1 -verify %s
// RUN: %cheri_purecap_cc1 -verify %s
// RUN: not %cheri_cc1 -ast-dump %s 2>/dev/null | FileCheck --check-prefix=AST %s
// RUN: not %cheri_purecap_cc1 -ast-dump %s 2>/dev/null | FileCheck --check-prefixes=AST,PURECAP-AST %s
//
// Various Sema tests for the __cheri_offset and __cheri_addr casts
//

void checkAST(char * __capability c) {
  // AST: |-FunctionDecl {{.+}} line:9:6 checkAST 'void (char * __capability)'
  long x1 = (__cheri_offset long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToOffset>
  // AST: -DeclRefExpr {{.+}} 'char * __capability' lvalue ParmVar {{.+}} 'c' 'char * __capability'

  long x2 = (__cheri_addr long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToAddress>
  // AST: -DeclRefExpr {{.+}} 'char * __capability' lvalue ParmVar {{.+}} 'c' 'char * __capability'
}

void types_offset(char * __capability c) {
  short x1 = (__cheri_offset short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the capability offset field}}
  unsigned short x2 = (__cheri_offset unsigned short)c; // expected-warning {{target type 'unsigned short' is smaller than the type 'long unsigned int' of the capability offset field}}
  long x3 = (__cheri_offset long)c;
  char * __capability x4 = (__cheri_offset char * __capability)c; // expected-error {{invalid target type 'char * __capability' for __cheri_offset}}
  long x5 = (__cheri_offset short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the capability offset field}}
  short x6 = (__cheri_offset long)c;
  char *x7 = (__cheri_offset char*)c; // expected-error-re {{invalid target type 'char *{{( __capability)?}}' for __cheri_offset}}
}

void types_addr(char * __capability c) {
  short x1 = (__cheri_addr short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the address}}
  unsigned short x2 = (__cheri_addr unsigned short)c; // expected-warning {{target type 'unsigned short' is smaller than the type 'long unsigned int' of the address}}
  long x3 = (__cheri_addr long)c;
  char * __capability x4 = (__cheri_addr char * __capability)c; // expected-error {{capability type 'char * __capability' is not a valid target type for __cheri_addr}}
  long x5 = (__cheri_addr short)c; // expected-warning {{target type 'short' is smaller than the type 'long int' of the address}}
  short x6 = (__cheri_addr long)c;
  char *x7 = (__cheri_addr char *)c;
#ifdef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 {{capability type 'char * __capability' is not a valid target type for __cheri_addr}}
#else
  // expected-error@-4 {{integral pointer type 'char *' is not a valid target type for __cheri_addr}}
#endif
}

#ifdef __CHERI_PURE_CAPABILITY__
void decay() {
  char buf[1];
  long x1 = (__cheri_addr long) buf;
  // PURECAP-AST: -FunctionDecl {{.*}} line:{{.+}} decay 'void ()'
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToAddress>
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'char * __capability' <ArrayToPointerDecay>
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'char [1]' lvalue Var {{.*}} 'buf' 'char [1]'
  long x2 = (__cheri_offset long) buf;
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToOffset>
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'char * __capability' <ArrayToPointerDecay>
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'char [1]' lvalue Var {{.*}} 'buf' 'char [1]'

}
#endif

// Test case from https://github.com/CTSRD-CHERI/clang/issues/184
// Should not cause any diagnostics:
struct malloc_type;

void    *kmalloc(unsigned long size, struct malloc_type *type, int flags);

void * __capability
malloc_c(unsigned long size, struct malloc_type *type, int flags)
{
        return ((__cheri_tocap void * __capability)kmalloc(size, type, flags));
}
