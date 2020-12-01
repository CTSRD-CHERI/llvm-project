// RUN: %cheri_cc1 %s -verify=expected,hybrid -ast-dump | FileCheck %s -check-prefixes AST,HYBRID-AST
// RUN: %cheri_purecap_cc1 %s -verify=expected,purecap -ast-dump | FileCheck %s -check-prefixes AST,PURECAP-AST
// purecap-no-diagnostics

void f() {
  // AST-LABEL: FunctionDecl
  // AST-SAME: referenced f 'void ()'
  char buf[1];
  char * __capability bufp;
  void (* __capability fp)();
  bufp = reinterpret_cast<char * __capability>(buf);
  // HYBRID-AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'char * __capability' reinterpret_cast<char * __capability> <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'char *' reinterpret_cast<char * __capability> <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'char *' <ArrayToPointerDecay> part_of_explicit_cast{{$}}
  fp = reinterpret_cast<void (* __capability)()>(f);
  // HYBRID-AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'void (* __capability)()' reinterpret_cast<void (* __capability)(void)> <PointerToCHERICapability>{{$}}
  // PURECAP-AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'void (*)()' reinterpret_cast<void (* __capability)(void)> <NoOp>{{$}}
  // AST-NEXT: ImplicitCastExpr {{.*}} {{.*}} 'void (*)()' <FunctionToPointerDecay> part_of_explicit_cast{{$}}
  bufp = static_cast<char * __capability>(buf);
  // hybrid-error@-1 {{static_cast from 'char *' to 'char * __capability' changes capability qualifier}}
  fp = static_cast<void (* __capability)()>(f);
  // hybrid-error@-1 {{static_cast from 'void (*)()' to 'void (* __capability)()' changes capability qualifier}}
}
