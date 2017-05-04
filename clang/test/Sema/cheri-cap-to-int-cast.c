// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

void* __capability a;
typedef __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__ vaddr_t;
typedef __attribute__((memory_address)) vaddr_t double_attribute;  // expected-warning {{attribute 'memory_address' is already applied}}
typedef __attribute__((memory_address)) __intcap_t err_cap_type;  // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses ('__intcap_t' is invalid)}}
typedef __attribute__((memory_address)) struct foo err_struct_type; // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses ('struct foo' is invalid)}}
typedef int* __attribute__((memory_address)) err_pointer_type; // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses ('int *' is invalid)}}

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
  unsigned long x1 = (unsigned long)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'unsigned long' is most likely an error}} expected-note{{insert cast to vaddr_t}}
  long x2 = (long)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}} expected-note{{insert cast to vaddr_t}}
  int x3 = (int)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'int' is most likely an error}} expected-note{{insert cast to vaddr_t}}
  ptrdiff_t x4 = (ptrdiff_t)a;  // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'ptrdiff_t' (aka 'long') is most likely an error}} expected-note{{insert cast to vaddr_t}}
  // These are okay
  uintptr_t x5 = (uintptr_t)a;
  intptr_t x6 = (intptr_t)a; 
  __uintcap_t x7 = (__uintcap_t)a;
  __intcap_t x8 = (__intcap_t)a; 

  vaddr_t x10 = (vaddr_t)a;
  other_addr_t x11 = (other_addr_t)a;
  void* __capability x12 = (void* __capability)a;
  int x13 = (int)(uintptr_t)a;
  int x14 = (int)(vaddr_t)a;
  word* __capability x15 = (word* __capability)a;
  long x16 = (long __attribute__((memory_address)))a; // no warning

#ifndef __CHERI_PURE_CAPABILITY__
  word* x17 = (word*)a; // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'word *' (aka '__uintcap_t *') is most likely an error}} expected-note{{insert cast to vaddr_t}}
#endif
}

void test_cheri_cast(void) {
  // __cheri_cast is a noop in pure ABI (but we still validate the parameter types)
  char c;
  struct test t;
  struct test *tptr;
  int * x = 0;
  (__cheri_cast char*)x; // expected-error{{invalid __cheri_cast from 'int *' to unrelated type 'char *'}}
  (__cheri_cast struct test*)t; // expected-error{{invalid source type 'struct test' for __cheri_cast: source must be a capability or a pointer}}
  (__cheri_cast struct test)tptr; // expected-error{{invalid target type 'struct test' for __cheri_cast: target must be a capability or a pointer}}
  (__cheri_cast struct test*)tptr; // this one is fine
  // TODO: warn in hybrid mode if cast is a noop (it will always be in purecap mode but we accept it so that both compile)
}
