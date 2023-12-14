// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify=expected,hybrid -ast-dump | FileCheck %s -check-prefix AST
// RUN: %cheri_purecap_cc1 -o - %s -fsyntax-only -verify=expected,purecap

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"
#pragma clang diagnostic warning "-Wpedantic"

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

#if !__has_extension(cheri_casts)
#error "cheri_casts feature should exist"
#endif


void* __capability a;
typedef __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__ ptraddr_t;
typedef __attribute__((memory_address)) ptraddr_t double_attribute;  // expected-warning {{attribute 'memory_address' is already applied}}
typedef __attribute__((memory_address)) __intcap err_cap_type;  // expected-error {{'memory_address' attribute only applies to integer types that can store addresses ('__intcap' is invalid)}}
typedef __attribute__((memory_address)) struct foo err_struct_type; // expected-error {{'memory_address' attribute only applies to integer types that can store addresses ('struct foo' is invalid)}}
typedef int* __attribute__((memory_address)) err_pointer_type; // expected-error {{'memory_address' attribute only applies to integer types that can store addresses}}

// seems like an attribute at the end is handled differently
// FIXME: unless I make this an error I get a crash in clang:
typedef unsigned __PTRDIFF_TYPE__ err_bad_location __attribute__((memory_address)); // expected-error {{'memory_address' type specifier must precede the declarator}}

/*
Assertion failed: ((attrs || DeclAttrs) && "no type attributes in the expected location!"), function fillAttributedTypeLoc, file /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp, line 4527.
Program received signal SIGABRT, Aborted.
0x000000081211435a in thr_kill () from /lib/libc.so.7
(gdb) bt
#0  0x000000081211435a in thr_kill () from /lib/libc.so.7
#1  0x0000000812114346 in raise () from /lib/libc.so.7
#2  0x00000008121142c9 in abort () from /lib/libc.so.7
#3  0x000000081217af91 in __assert () from /lib/libc.so.7
#4  0x0000000816e14732 in fillAttributedTypeLoc (TL=..., attrs=0x0, DeclAttrs=0x0)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp:4526
#5  0x0000000816e2721f in (anonymous namespace)::TypeSpecLocFiller::VisitAttributedTypeLoc
    (this=0x7fffffff70c8, TL=...)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp:4572
#6  0x0000000816e15e9a in clang::TypeLocVisitor<(anonymous namespace)::TypeSpecLocFiller, void>::Visit (this=0x7fffffff70c8, TyLoc=...)
    at /home/alr48/cheri/sources/llvm/tools/clang/include/clang/AST/TypeNodes.def:94
#7  0x0000000816e145da in clang::Sema::GetTypeSourceInfoForDeclarator (this=0x818ce7000,
    D=..., T=..., ReturnTypeInfo=0x0)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp:4915
#8  0x0000000816e13d09 in GetFullTypeForDeclarator (state=..., declSpecType=...,
    TInfo=0x0) at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp:4315
#9  0x0000000816e0eab5 in clang::Sema::GetTypeForDeclarator (this=0x818ce7000, D=...,
    S=0x818d15700)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaType.cpp:4335
#10 0x00000008166d00ee in clang::Sema::HandleDeclarator (this=0x818ce7000, S=0x818d15700,
    D=..., TemplateParamLists=...)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaDecl.cpp:4780
#11 0x00000008166cfa8b in clang::Sema::ActOnDeclarator (this=0x818ce7000, S=0x818d15700,
    D=...) at /home/alr48/cheri/sources/llvm/tools/clang/lib/Sema/SemaDecl.cpp:4579
#12 0x00000008160cbbba in clang::Parser::ParseDeclarationAfterDeclaratorAndAttributes (
    this=0x818c86000, D=..., TemplateInfo=..., FRI=0x0)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Parse/ParseDecl.cpp:1934
#13 0x00000008160ca85c in clang::Parser::ParseDeclGroup (this=0x818c86000, DS=...,
    Context=0, DeclEnd=0x7fffffffa1b8, FRI=0x0)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Parse/ParseDecl.cpp:1809
#14 0x00000008160c5d67 in clang::Parser::ParseSimpleDeclaration (this=0x818c86000,
    Context=0, DeclEnd=..., Attrs=..., RequireSemi=true, FRI=0x0)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Parse/ParseDecl.cpp:1541
#15 0x00000008160c5acd in clang::Parser::ParseDeclaration (this=0x818c86000, Context=0,
    DeclEnd=..., attrs=...)
    at /home/alr48/cheri/sources/llvm/tools/clang/lib/Parse/ParseDecl.cpp:1486
*/
typedef const ptraddr_t other_addr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef unsigned __intcap uintptr_t;
typedef __intcap intptr_t;
typedef uintptr_t word;
struct test {
  int x;
};

void foo(void) {
  unsigned long x1 = (unsigned long)a;
  // hybrid-warning@-1{{cast from capability type 'void * __capability' to non-capability, non-address type 'unsigned long' is most likely an error}}
  // purecap-warning@-2{{cast from capability type 'void * __capability' (aka 'void *') to non-capability, non-address type 'unsigned long' is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'unsigned long' <PointerToIntegral>

  long x2 = (long)a;
  // hybrid-warning@-1{{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  // purecap-warning@-2{{cast from capability type 'void * __capability' (aka 'void *') to non-capability, non-address type 'long' is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'long' <PointerToIntegral>

  int x3 = (int)a;
  // hybrid-warning@-1{{cast from capability type 'void * __capability' to non-capability, non-address type 'int' is most likely an error}}
  // purecap-warning@-2{{cast from capability type 'void * __capability' (aka 'void *') to non-capability, non-address type 'int' is most likely an error}}
  // hybrid-warning@-3{{cast to smaller integer type 'int' from 'void * __capability'}}
  // purecap-warning@-4{{cast to smaller integer type 'int' from 'void * __capability' (aka 'void *')}}
  // AST: CStyleCastExpr {{.+}} 'int' <PointerToIntegral>

  ptrdiff_t x4 = (ptrdiff_t)a;
  // hybrid-warning@-1{{cast from capability type 'void * __capability' to non-capability, non-address type 'ptrdiff_t' (aka 'long') is most likely an error}}
  // purecap-warning@-2{{cast from capability type 'void * __capability' (aka 'void *') to non-capability, non-address type 'ptrdiff_t' (aka 'long') is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'ptrdiff_t':'long' <PointerToIntegral>

  // These are okay
  uintptr_t x5 = (uintptr_t)a;
  // AST: CStyleCastExpr {{.+}} 'uintptr_t':'unsigned __intcap' <PointerToIntegral>
  intptr_t x6 = (intptr_t)a;
  // AST: CStyleCastExpr {{.+}} 'intptr_t':'__intcap' <PointerToIntegral>
  unsigned __intcap x7 = (unsigned __intcap)a;
  // AST: CStyleCastExpr {{.+}} 'unsigned __intcap' <PointerToIntegral>
  __intcap x8 = (__intcap)a; 
  // AST: CStyleCastExpr {{.+}} '__intcap' <PointerToIntegral>

  ptraddr_t x10 = (ptraddr_t)a;
  // AST: CStyleCastExpr {{.+}} 'ptraddr_t':'unsigned long' <PointerToIntegral>
  other_addr_t x11 = (other_addr_t)a;
  // AST: CStyleCastExpr {{.+}} 'ptraddr_t':'unsigned long' <PointerToIntegral>
  void* __capability x12 = (void* __capability)a;
  // AST: CStyleCastExpr {{.+}} 'void * __capability':'void * __capability' <NoOp>
  int x13 = (int)(uintptr_t)a;  // TODO: later on this should probably also be an error
  // AST: CStyleCastExpr {{.+}} 'int' <IntegralCast>
  // AST-NEXT: CStyleCastExpr {{.+}} 'uintptr_t':'unsigned __intcap' <PointerToIntegral>
  int x14 = (int)(ptraddr_t)a;
  // AST: CStyleCastExpr {{.+}} 'int' <IntegralCast>
  // AST-NEXT: CStyleCastExpr {{.+}} 'ptraddr_t':'unsigned long' <PointerToIntegral>
  word* __capability x15 = (word* __capability)a;
  // AST: CStyleCastExpr {{.+}} 'word * __capability':'word * __capability' <BitCast>
  long x16 = (long __attribute__((memory_address)))a; // no warning
  // AST: CStyleCastExpr {{.+}} 'long __attribute__((memory_address))':'long' <PointerToIntegral>

#ifndef __CHERI_PURE_CAPABILITY__
  word* x17 = (word*)a; // expected-error {{cast from capability type 'void * __capability' to non-capability type 'word *' (aka 'unsigned __intcap *') is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'word *' <CHERICapabilityToPointer>
#endif
}

void test_cheri_to_from_cap(void) {
  // __cheri_tocap and __cheri_fromcap are no-ops if source and destination
  // types are capabilities (but we still validate the types)
  char c;
  struct test t;
  struct test *tptr;
  int * x = 0;
  (__cheri_tocap char* __capability)x;
  // hybrid-error@-1{{invalid __cheri_tocap from 'int *' to unrelated type 'char * __capability'}}
  // purecap-error@-2{{invalid __cheri_tocap from 'int *' to unrelated type 'char * __capability' (aka 'char *')}}
  (__cheri_tocap struct test* __capability)t; // expected-error{{invalid source type 'struct test' for __cheri_tocap: source must be a pointer}}
  (__cheri_tocap struct test)tptr; // expected-error{{invalid target type 'struct test' for __cheri_tocap: target must be a capability}}
  (__cheri_fromcap struct test*)t; // expected-error{{invalid source type 'struct test' for __cheri_fromcap: source must be a capability}}

#ifdef __CHERI_PURE_CAPABILITY__
  // Check no-op warning when in purecap
  (void)(__cheri_fromcap struct test*)tptr;
  // expected-warning@-1 {{__cheri_fromcap from 'struct test *' to 'struct test *' is a no-op}}
#endif
  int* __capability intptr_to_cap = (__cheri_tocap int* __capability)x;
  // purecap-warning@-1 {{__cheri_tocap from 'int *' to 'int * __capability' (aka 'int *') is a no-op}}
#ifndef __CHERI_PURE_CAPABILITY__
  // Check no-op warning when in hybrid ABI
  int* __capability intptr_to_cap2 = (__cheri_tocap int* __capability)intptr_to_cap;
  // expected-warning@-1 {{__cheri_tocap from 'int * __capability' to 'int * __capability' is a no-op}}
#endif
  // AST: CStyleCastExpr {{.+}} 'int * __capability':'int * __capability' <PointerToCHERICapability>
  const int* __capability const_intcap = (__cheri_tocap int* __capability)x;
  // purecap-warning@-1 {{__cheri_tocap from 'int *' to 'int * __capability' (aka 'int *') is a no-op}}
  // AST: ImplicitCastExpr {{.+}} 'const int * __capability':'const int * __capability' <NoOp>
  // AST-NEXT: CStyleCastExpr {{.+}} 'int * __capability':'int * __capability' <PointerToCHERICapability>
  (__cheri_tocap int* __capability)const_intcap;
  // hybrid-error@-1{{invalid __cheri_tocap from 'const int * __capability' to unrelated type 'int * __capability'}}
  // purecap-error@-2{{invalid __cheri_tocap from 'const int * __capability' (aka 'const int *') to unrelated type 'int * __capability' (aka 'int *')}}
  (__cheri_fromcap int* __capability)const_intcap;
  // hybrid-error@-1{{invalid __cheri_fromcap from 'const int * __capability' to unrelated type 'int * __capability'}}
  // purecap-error@-2{{invalid __cheri_fromcap from 'const int * __capability' (aka 'const int *') to unrelated type 'int * __capability' (aka 'int *')}}
}
