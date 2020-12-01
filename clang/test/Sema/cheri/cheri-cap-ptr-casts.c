// RUN: %cheri_cc1 %s -verify=expected,hybrid,hybrid-c -ast-dump | FileCheck %s -check-prefixes AST,HYBRID-AST
// RUN: %cheri_cc1 -x c++ %s -verify=expected,hybrid,hybrid-cxx -ast-dump | FileCheck %s -check-prefixes AST,HYBRID-AST
// RUN: %cheri_purecap_cc1 %s -verify=expected,purecap -ast-dump | FileCheck %s -check-prefixes AST,PURECAP-AST
// RUN: %cheri_purecap_cc1 -x c++ %s -verify=expected,purecap -ast-dump | FileCheck %s -check-prefixes AST,PURECAP-AST

// XXX: Currently we only get diagnostics for hybrid as DiagnoseCHERIPtr only
// looks through integer pointers, and is only called for C, not C++. This
// should also not be a CHERI-specific error, rather just another -Wcast-align
// warning given faulting on unaligned accesses is nothing new to CHERI (this
// isn't a warning about tag loss).
// purecap-no-diagnostics
// hybrid-cxx-no-diagnostics
void a() {
  // AST-LABEL: FunctionDecl
  // AST-SAME: a 'void ()'
  unsigned long foo[8];
  ((int * __capability *)foo)[0] = 0;
  // hybrid-c-error-re@-1 {{cast from 'unsigned long *' to 'int * __capability *' increases required alignment from 8 to {{16|32}}}}
  // hybrid-c-note@-2 {{use __builtin_assume_aligned(..., sizeof(void* __capability)) if you know that the source type is sufficiently aligned}}
  // HYBRID-AST:  CStyleCastExpr {{.*}} {{.*}} 'int * __capability *' <BitCast>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'int **' <BitCast>{{$}}
  // AST-NEXT:    ImplicitCastExpr {{.*}} {{.*}} 'unsigned long *' <ArrayToPointerDecay> part_of_explicit_cast{{$}}
}

void f() {
  // AST-LABEL: FunctionDecl
  // AST-SAME: referenced f 'void ()'
  char buf[1];
  char * __capability bufp;
  void (* __capability fp)();
  bufp = (__cheri_tocap char * __capability) buf;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'char *' <ArrayToPointerDecay> part_of_explicit_cast{{$}}
  fp = (__cheri_tocap void (* __capability)()) f;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void (* __capability)()' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void (*)()' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'void (*)()' <FunctionToPointerDecay> part_of_explicit_cast{{$}}
}

void fromcap(void *__capability voidcap) {
  // AST-LABEL: FunctionDecl
  // HYBRID-AST-SAME: fromcap 'void (void * __capability)'
  // PURECAP-AST-SAME: fromcap 'void (void *)'

  char * __capability x;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <CHERICapabilityToPointer>{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'char * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: DeclRefExpr {{.*}} 'char * __capability' lvalue Var {{.+}} 'x' 'char * __capability'{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <NoOp>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <LValueToRValue> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: DeclRefExpr {{.*}} 'char *' lvalue Var {{.+}} 'x' 'char *'{{$}}
  char *y = (__cheri_fromcap char *)x;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <CHERICapabilityToPointer>{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'void * __capability' <BitCast> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'char * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: DeclRefExpr {{.*}} 'char * __capability' lvalue Var {{.+}} 'x' 'char * __capability'{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <NoOp>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <BitCast> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <LValueToRValue> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: DeclRefExpr {{.*}} 'char *' lvalue Var {{.+}} 'x' 'char *'{{$}}
  void *z = (__cheri_fromcap void *)x;

  // Check that we insert the appropriate bitcasts in the purecap ABI
  void *p1 = (__cheri_fromcap void *)voidcap;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <CHERICapabilityToPointer>{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'void * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: DeclRefExpr {{.*}} 'void * __capability' lvalue ParmVar {{.+}} 'voidcap' 'void * __capability'{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <NoOp>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <LValueToRValue> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: DeclRefExpr {{.*}} 'void *' lvalue ParmVar {{.+}} 'voidcap' 'void *'{{$}}
  char *p2 = (__cheri_fromcap char *)voidcap;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <CHERICapabilityToPointer>{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'char * __capability' <BitCast> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.*}} 'void * __capability' <LValueToRValue> part_of_explicit_cast{{$}}
  // HYBRID-AST-NEXT: DeclRefExpr {{.*}} 'void * __capability' lvalue ParmVar {{.+}} 'voidcap' 'void * __capability'{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <NoOp>{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <BitCast> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <LValueToRValue> part_of_explicit_cast{{$}}
  // PURECAP-AST-NEXT: DeclRefExpr {{.*}} 'void *' lvalue ParmVar {{.+}} 'voidcap' 'void *'{{$}}
}

void tocap(void * voidptr) {
  // AST-LABEL: FunctionDecl
  // AST-SAME: tocap 'void (void *)'
  char *x;

  char * __capability y = (__cheri_tocap char * __capability)x;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: DeclRefExpr {{.*}} 'char *' lvalue Var {{.+}} 'x' 'char *'{{$}}
  void * __capability z = (__cheri_tocap void * __capability)x;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <BitCast> part_of_explicit_cast{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: DeclRefExpr {{.*}} 'char *' lvalue Var {{.+}} 'x' 'char *'{{$}}

  void * __capability p1 = (__cheri_tocap void * __capability)voidptr;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: DeclRefExpr {{.*}} 'void *' lvalue ParmVar {{.+}} 'voidptr' 'void *'{{$}}
  char * __capability p2 = (__cheri_tocap char * __capability)voidptr;
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'char * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'char *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'char *' <BitCast> part_of_explicit_cast{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: DeclRefExpr {{.*}} 'void *' lvalue ParmVar {{.+}} 'voidptr' 'void *'{{$}}
}

// https://github.com/CTSRD-CHERI/clang/issues/178
struct a {
  void *__capability ptr;
};
struct a noop_void_cast(struct a *arg) {
  // AST-LABEL: FunctionDecl
  // AST-SAME: noop_void_cast 'struct a (struct a *)'

  struct a first;
  first.ptr = (__cheri_tocap void *__capability)arg;
  // The following used to generate a NoOp cast that changed types in the purecap ABI (and then caused codegen to crash):
  // HYBRID-AST: CStyleCastExpr {{.*}} {{.*}} 'void * __capability' <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CStyleCastExpr {{.*}} {{.*}} 'void *' <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'void *' <BitCast> part_of_explicit_cast{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} 'struct a *' <LValueToRValue> part_of_explicit_cast{{$}}
  // AST-NEXT: DeclRefExpr {{.*}} 'struct a *' lvalue ParmVar {{.+}} 'arg' 'struct a *'{{$}}
  return first;
}
