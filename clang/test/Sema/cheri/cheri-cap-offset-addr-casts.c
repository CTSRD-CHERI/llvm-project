// RUN: %cheri_cc1 -verify=expected,hybrid %s
// RUN: %cheri_purecap_cc1 -verify=expected,purecap %s
// RUN: not %cheri_cc1 -ast-dump %s 2>/dev/null | FileCheck --check-prefix=AST %s
// RUN: not %cheri_purecap_cc1 -ast-dump %s 2>/dev/null | FileCheck --check-prefixes=AST,PURECAP-AST %s
//
// Various Sema tests for the __cheri_offset and __cheri_addr casts
//

void checkAST(char * __capability c) {
  // AST: |-FunctionDecl {{.+}} line:9:6 checkAST 'void (char * __capability)'
  long x1 = (__cheri_offset long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToOffset>{{$}}
  // The part_of_explicit_cast was not being set previously
  // AST-NEXT: ImplicitCastExpr {{.*}} <col:34> 'char * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: -DeclRefExpr {{.+}} 'char * __capability' lvalue ParmVar {{.+}} 'c' 'char * __capability'{{$}}

  long x2 = (__cheri_addr long)c;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'long' <CHERICapabilityToAddress>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} <col:32> 'char * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: -DeclRefExpr {{.+}} 'char * __capability' lvalue ParmVar {{.+}} 'c' 'char * __capability'{{$}}
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

struct foo { char* x; };

void bad_src_type(long l, struct foo foo, struct foo* fooptr) {
  long a1 = (__cheri_addr long)l; // expected-error{{invalid source type 'long' for __cheri_addr: source must be a capability}}
  long a2 = (__cheri_addr long)foo;  // expected-error{{invalid source type 'struct foo' for __cheri_addr: source must be a capability}}
  // These are fine (both hybrid and purecap):
  long a3 = (__cheri_addr long)foo.x;
  long a4 = (__cheri_addr long)fooptr;

  // However, for __cheri_offset the source must be a capability
  long o1 = (__cheri_offset long)l;    // expected-error{{invalid source type 'long' for __cheri_offset: source must be a capability}}
  long o2 = (__cheri_offset long)foo;  // expected-error{{invalid source type 'struct foo' for __cheri_offset: source must be a capability}}
  // These are errors in the hybrid ABI::
  long o3 = (__cheri_offset long)foo.x; // hybrid-error{{invalid source type 'char *' for __cheri_offset: source must be a capability}}
  long o4 = (__cheri_offset long)fooptr; // hybrid-error{{invalid source type 'struct foo *' for __cheri_offset: source must be a capability}}
}


#ifdef __CHERI_PURE_CAPABILITY__
void decay(void) {
  char buf[1];
  long x1 = (__cheri_addr long) buf;
  // PURECAP-AST: -FunctionDecl {{.*}} line:[[@LINE-3]]:6 referenced decay 'void (void)'
  // PURECAP-AST: CStyleCastExpr {{.*}} <col:13, col:33> 'long' <CHERICapabilityToAddress>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} <col:33> 'char * __capability' <ArrayToPointerDecay> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'char [1]' lvalue Var {{.*}} 'buf' 'char [1]'{{$}}
  long x2 = (__cheri_offset long) buf;
  // PURECAP-AST: CStyleCastExpr {{.*}} <col:13, col:35> 'long' <CHERICapabilityToOffset>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} <col:35> 'char * __capability' <ArrayToPointerDecay> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'char [1]' lvalue Var {{.*}} 'buf' 'char [1]'{{$}}

  // Also check function-to-pointer decay:
  long x3 = (__cheri_addr long) decay;
  // PURECAP-AST: CStyleCastExpr {{.*}} <col:13, col:33> 'long' <CHERICapabilityToAddress>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} <col:33> 'void (* __capability)(void)' <FunctionToPointerDecay> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'void (void)' Function {{.+}} 'decay' 'void (void)'{{$}}
  long x4 = (__cheri_offset long) decay;
  // PURECAP-AST: CStyleCastExpr {{.*}} <col:13, col:35> 'long' <CHERICapabilityToOffset>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} <col:35> 'void (* __capability)(void)' <FunctionToPointerDecay> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: -DeclRefExpr {{.*}} 'void (void)' Function {{.+}} 'decay' 'void (void)'{{$}}
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
