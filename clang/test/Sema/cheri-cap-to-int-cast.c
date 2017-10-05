// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify
// RUN: not %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -ast-dump | FileCheck %s -check-prefix AST
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -o - %s -fsyntax-only -verify

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"
#pragma clang diagnostic warning "-Wpedantic"

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

#if !__has_extension(__cheri_cast)
#error "__cheri_cast feature should exist"
#endif


void* __capability a;
typedef __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__ vaddr_t;
typedef __attribute__((memory_address)) vaddr_t double_attribute;  // expected-warning {{attribute 'memory_address' is already applied}}
typedef __attribute__((memory_address)) __intcap_t err_cap_type;  // expected-error {{'memory_address' attribute only applies to integer types that can store addresses ('__intcap_t' is invalid)}}
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
typedef const vaddr_t other_addr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __uintcap_t uintptr_t;
typedef __intcap_t intptr_t;
typedef uintptr_t word;
struct test {
  int x;
};

void foo(void) {
  unsigned long x1 = (unsigned long)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'unsigned long' is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'unsigned long' <PointerToIntegral>

  long x2 = (long)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'long' <PointerToIntegral>

  int x3 = (int)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'int' is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'int' <PointerToIntegral>

  ptrdiff_t x4 = (ptrdiff_t)a;  // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'ptrdiff_t' (aka 'long') is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'ptrdiff_t':'long' <PointerToIntegral>

  // These are okay
  uintptr_t x5 = (uintptr_t)a;
  // AST: CStyleCastExpr {{.+}} 'uintptr_t':'__uintcap_t' <PointerToIntegral>
  intptr_t x6 = (intptr_t)a;
  // AST: CStyleCastExpr {{.+}} 'intptr_t':'__intcap_t' <PointerToIntegral>
  __uintcap_t x7 = (__uintcap_t)a;
  // AST: CStyleCastExpr {{.+}} '__uintcap_t':'__uintcap_t' <PointerToIntegral>
  __intcap_t x8 = (__intcap_t)a; 
  // AST: CStyleCastExpr {{.+}} '__intcap_t':'__intcap_t' <PointerToIntegral>

  vaddr_t x10 = (vaddr_t)a;
  // AST: CStyleCastExpr {{.+}} 'vaddr_t':'unsigned long' <PointerToIntegral>
  other_addr_t x11 = (other_addr_t)a;
  // AST: CStyleCastExpr {{.+}} 'vaddr_t':'unsigned long' <PointerToIntegral>
  void* __capability x12 = (void* __capability)a;
  // AST: CStyleCastExpr {{.+}} 'void * __capability' <NoOp>
  int x13 = (int)(uintptr_t)a;  // TODO: later on this should probably also be an error
  // AST: CStyleCastExpr {{.+}} 'int' <IntegralCast>
  // AST-NEXT: CStyleCastExpr {{.+}} 'uintptr_t':'__uintcap_t' <PointerToIntegral>
  int x14 = (int)(vaddr_t)a;
  // AST: CStyleCastExpr {{.+}} 'int' <IntegralCast>
  // AST-NEXT: CStyleCastExpr {{.+}} 'vaddr_t':'unsigned long' <PointerToIntegral>
  word* __capability x15 = (word* __capability)a;
  // AST: CStyleCastExpr {{.+}} 'word * __capability' <BitCast>
  long x16 = (long __attribute__((memory_address)))a; // no warning
  // AST: CStyleCastExpr {{.+}} 'long __attribute__((memory_address))':'long' <PointerToIntegral>

#ifndef __CHERI_PURE_CAPABILITY__
  word* x17 = (word*)a; // expected-error {{cast from capability type 'void * __capability' to non-capability type 'word *' (aka '__uintcap_t *') is most likely an error}}
  // AST: CStyleCastExpr {{.+}} 'word *' <CHERICapabilityToPointer>
#endif
}

void test_cheri_cast(void) {
  // __cheri_cast is a noop in pure ABI (but we still validate the parameter types)
  char c;
  struct test t;
  struct test *tptr;
  int * x = 0;
  (__cheri_cast char*)x;
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-error@-2{{invalid __cheri_cast from 'int *' to unrelated type 'char *'}}
#else
  // expected-error@-4{{invalid __cheri_cast from 'int * __capability' to unrelated type 'char * __capability'}}
#endif
  (__cheri_cast struct test*)t; // expected-error{{invalid source type 'struct test' for __cheri_cast: source must be a capability or a pointer}}
  (__cheri_cast struct test)tptr; // expected-error{{invalid target type 'struct test' for __cheri_cast: target must be a capability or a pointer}}
  (void)(__cheri_cast struct test*)tptr;
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-warning@-2 {{__cheri_cast from 'struct test *' to 'struct test *' is a noop}}
#endif

  int* __capability intptr_to_cap = (__cheri_cast int* __capability)x;
  // AST: CStyleCastExpr {{.+}} 'int * __capability' <PointerToCHERICapability>
  const int* __capability const_intcap = (__cheri_cast int* __capability)x;
  // AST: ImplicitCastExpr {{.+}} 'const int * __capability' <BitCast>
  // AST-NEXT: CStyleCastExpr {{.+}} 'int * __capability' <PointerToCHERICapability>
  (__cheri_cast int* __capability)const_intcap; // expected-error{{invalid __cheri_cast from 'const int * __capability' to unrelated type 'int * __capability'}}
}
