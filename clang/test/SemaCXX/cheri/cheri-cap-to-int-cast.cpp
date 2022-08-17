// RUN: %cheri_cc1 -std=c++11 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_cc1 -std=c++11 -o - %s -fsyntax-only -ast-dump -verify=expected,hybrid 2>&1 | FileCheck -check-prefixes=AST,HYBRID-AST %s
// RUN: not %cheri_purecap_cc1 -std=c++11 -o - %s -fsyntax-only -ast-dump 2>&1 | FileCheck -check-prefixes=AST,PURECAP-AST %s
// RUN: %cheri_cc1 -std=c++11 -target-abi purecap -o - %s -fsyntax-only -verify=expected,purecap

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

#if !__has_extension(cheri_casts)
#error "cheri_casts feature should exist"
#endif

void *__capability a;
using ptraddr_t = __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__;
using double_attribute = __attribute__((memory_address)) ptraddr_t;   // expected-warning {{attribute 'memory_address' is already applied}}
using err_cap_type = __attribute__((memory_address)) __intcap;    // expected-error {{'memory_address' attribute only applies to integer types that can store addresses ('__intcap' is invalid)}}
using err_struct_type = __attribute__((memory_address)) struct foo; // expected-error {{'memory_address' attribute only applies to integer types that can store addresses ('struct foo' is invalid)}}
using err_pointer_type = int *__attribute__((memory_address));      // expected-error {{'memory_address' attribute only applies to integer types that can store addresses}}

typedef const ptraddr_t other_addr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef unsigned __intcap uintptr_t;
typedef __intcap intptr_t;
typedef uintptr_t word;
struct test {
  int x;
};

void cast_vaddr() {
  // These casts should be fine, typedef with attribute
  ptraddr_t v = reinterpret_cast<ptraddr_t>(a);
  v = static_cast<ptraddr_t>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'ptraddr_t' (aka 'unsigned long') is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'ptraddr_t' (aka 'unsigned long') is not allowed}}
  v = (ptraddr_t)a;
  v = ptraddr_t(a);
  v = ptraddr_t{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void *'}}
}

void cast_vaddr2() {
  // These casts should be fine, typedef with attribute
  long __attribute__((memory_address)) v = reinterpret_cast<long __attribute__((memory_address))>(a);
  v = static_cast<long __attribute__((memory_address))>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'long __attribute__((memory_address))' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'long __attribute__((memory_address))' is not allowed}}
  v = (long __attribute__((memory_address)))a;
  // these won't work, need single token for functional/brace-init:
  // v = long __attribute__((memory_address))(a);
  // v = long __attribute__((memory_address)){a};

  __attribute__((memory_address)) long v2 = reinterpret_cast<__attribute__((memory_address)) long>(a);
  v2 = static_cast<__attribute__((memory_address)) long>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'long __attribute__((memory_address))' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'long __attribute__((memory_address))' is not allowed}}
  v2 = (__attribute__((memory_address)) long)a;
}

void cast_long() {
  long v = reinterpret_cast<long>(a);
  // hybrid-warning@-1 {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  // purecap-warning@-2 {{cast from capability type 'void *' to non-capability, non-address type 'long' is most likely an error}}
  v = static_cast<long>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'long' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'long' is not allowed}}
  v = (long)a;
  // hybrid-warning@-1 {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  // purecap-warning@-2 {{cast from capability type 'void *' to non-capability, non-address type 'long' is most likely an error}}
  v = long(a);
  // hybrid-warning@-1 {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  // purecap-warning@-2 {{cast from capability type 'void *' to non-capability, non-address type 'long' is most likely an error}}
  v = long{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'long' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'long' with an lvalue of type 'void *'}}
}

// Should cause narrowing errors
void cast_int() {
  int v = reinterpret_cast<int>(a); // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = static_cast<int>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'int' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'int' is not allowed}}
  v = (int)a; // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = int(a); // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = int{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'int' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'int' with an lvalue of type 'void *'}}
}

void cast_uintcap() {
  // AST-LABEL: FunctionDecl {{.+}} cast_uintcap 'void ()'
  unsigned __intcap v = reinterpret_cast<unsigned __intcap>(a);
  // AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'unsigned __intcap' reinterpret_cast<unsigned __intcap> <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  v = reinterpret_cast<unsigned __intcap>(nullptr);
  // AST: CXXReinterpretCastExpr {{.*}} {{.*}} 'unsigned __intcap __attribute__((cheri_no_provenance))':'unsigned __intcap' reinterpret_cast<unsigned __intcap> <PointerToIntegral>
  // AST-NEXT: CXXNullPtrLiteralExpr {{.+}} 'nullptr_t'
  v = static_cast<unsigned __intcap>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to 'unsigned __intcap' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to 'unsigned __intcap' is not allowed}}
  v = (unsigned __intcap)a;
  // AST: CStyleCastExpr {{.*}} {{.*}} 'unsigned __intcap' <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  // NB: C++ only supports a single word type name for functional casts and
  // direct list initialisers (GNU C++ supports compound literals for the
  // latter) so we need to introduce a single word alias for testing (we could
  // use the compiler-defined __uintcap_t but this more closely matches the
  // tests in cast_intcap).
  using __uintcap = unsigned __intcap;
  v = __uintcap(a);
  // AST: CXXFunctionalCastExpr {{.*}} {{.*}} '__uintcap':'unsigned __intcap' functional cast to __uintcap <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  v = __uintcap{a};
  // hybrid-error@-1 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void *'}}
}

void cast_intcap() {
  // AST-LABEL: FunctionDecl {{.+}} cast_intcap 'void ()'
  __intcap v = reinterpret_cast<__intcap>(a);
  // AST: CXXReinterpretCastExpr {{.*}} {{.*}} '__intcap' reinterpret_cast<__intcap> <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  v = reinterpret_cast<__intcap>(nullptr);
  // AST: CXXReinterpretCastExpr {{.*}} {{.*}} '__intcap __attribute__((cheri_no_provenance))':'__intcap' reinterpret_cast<__intcap> <PointerToIntegral>
  // AST-NEXT: CXXNullPtrLiteralExpr {{.+}} 'nullptr_t'
  v = static_cast<__intcap>(a);
  // hybrid-error@-1 {{static_cast from 'void * __capability' to '__intcap' is not allowed}}
  // purecap-error@-2 {{static_cast from 'void *' to '__intcap' is not allowed}}
  v = (__intcap)a;
  // AST: CStyleCastExpr {{.*}} {{.*}} '__intcap' <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  v = __intcap(a);
  // AST: CXXFunctionalCastExpr {{.*}} {{.*}} '__intcap' functional cast to __intcap <PointerToIntegral>
  // HYBRID-AST-NEXT: ImplicitCastExpr {{.+}} 'void * __capability' <LValueToRValue>
  // HYBRID-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void * __capability'
  // PURECAP-AST-NEXT: ImplicitCastExpr {{.+}} 'void *' <LValueToRValue>
  // PURECAP-AST-NEXT: DeclRefExpr {{.+}} 'a' 'void *'
  v = __intcap{a};
  // hybrid-error@-1 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void *'}}
}

void cast_to_cap(void) {
  // AST-LABEL: FunctionDecl {{.+}} cast_to_cap 'void ()'
  // noops that shouldn't warn
  auto p1 = reinterpret_cast<void * __capability>(a); // no warning
  auto p2 = (void *__capability)a;                    // no warning

  auto w1 = reinterpret_cast<word * __capability>(a); // no warning
  auto w2 = (word * __capability) a;                  // no warning
}

// Currently none of these warn: https://github.com/CTSRD-CHERI/clang/issues/140

void probably_an_error(unsigned __intcap *cap) {
  long l = *cap;
  l += 1;
  *cap = l;
}

void check_uintcap_to_int() {
  unsigned __intcap cap;
  int i = cap;
  i = (int)cap;
  i = int(cap);
  i = int{cap}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'int' in initializer list}} \
                // expected-note {{insert an explicit cast to silence this issue}}
  i = static_cast<int>(cap);
  long l = cap;
  l = (long)cap;
  l = long(cap);
  l = long{cap}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'long' in initializer list}} \
                 // expected-note {{insert an explicit cast to silence this issue}}
  l = static_cast<long>(cap);

  i = reinterpret_cast<int>(cap);  // expected-error {{reinterpret_cast from 'unsigned __intcap' to 'int' is not allowed}}
  l = reinterpret_cast<long>(cap); // expected-error {{reinterpret_cast from 'unsigned __intcap' to 'long' is not allowed}}
}

class test_class {};

#define DO_ALL_CASTS(to, from)           \
  do {                                   \
    to var = reinterpret_cast<to>(from); \
    var = static_cast<to>(from);         \
    var = const_cast<to>(from);          \
    var = dynamic_cast<to>(from);        \
    var = (to)from;                      \
    var = to(from);                      \
    var = to{from};                      \
  } while (false)

void cast_ptr() {
  // XXXAR: these should also warn
  using voidp = void *;
  DO_ALL_CASTS(voidp, a);
  // hybrid-error@-1 3{{cast from capability type 'void * __capability' to non-capability type 'voidp' (aka 'void *') is most likely an error}}
  // hybrid-error@-2 {{converting capability type 'void * __capability' to non-capability type 'voidp' (aka 'void *') without an explicit cast; if this is intended use __cheri_fromcap}}
  // hybrid-error@-3 {{const_cast from 'void * __capability' to 'voidp' (aka 'void *') is not allowed}}
  // hybrid-error@-4 {{static_cast from 'void * __capability' to 'voidp' (aka 'void *') changes capability qualifier}}
  // expected-error@-5 {{'void' is not a class}}

  using wordp = word *;
  DO_ALL_CASTS(wordp, a);
  // hybrid-error@-1 3 {{cast from capability type 'void * __capability' to non-capability type 'wordp' (aka 'unsigned __intcap *') is most likely an error}}
  // hybrid-error@-2 {{cannot implicitly or explicitly convert non-capability type 'void * __capability' to unrelated capability type 'wordp' (aka 'unsigned __intcap *')}}
  // hybrid-error@-3 {{const_cast from 'void * __capability' to 'wordp' (aka 'unsigned __intcap *') is not allowed}}
  // purecap-error@-4 {{const_cast from 'void *' to 'wordp' (aka 'unsigned __intcap *') is not allowed}}
  // purecap-error@-5 {{cannot initialize a value of type 'wordp' (aka 'unsigned __intcap *') with an lvalue of type 'void *'}}
  // expected-error@-6 {{'unsigned __intcap' is not a class}}
  // hybrid-error@-7 {{static_cast from 'void * __capability' to 'wordp' (aka 'unsigned __intcap *') changes capability qualifier}}

  using test_class_ptr = test_class *;
  test_class *__capability b = nullptr;
  DO_ALL_CASTS(test_class_ptr, b);
  // hybrid-error@-1 3 {{cast from capability type 'test_class * __capability' to non-capability type 'test_class_ptr' (aka 'test_class *') is most likely an error}}
  // hybrid-error@-2 {{converting capability type 'test_class * __capability' to non-capability type 'test_class_ptr' (aka 'test_class *') without an explicit cast; if this is intended use __cheri_fromcap}}
  // hybrid-error@-3 {{static_cast from 'test_class * __capability' to 'test_class_ptr' (aka 'test_class *') changes capability qualifier}}
  // hybrid-error@-4 {{const_cast from 'test_class * __capability' to 'test_class_ptr' (aka 'test_class *') is not allowed}}
  // hybrid-error@-5 {{dynamic_cast from 'test_class * __capability' to 'test_class *' changes capability qualifier}}
}

#ifndef __CHERI_PURE_CAPABILITY__
void cast_ref(int &__capability cap_ref) {
  int x;
  using intref = int &;
  int &v = reinterpret_cast<int &>(cap_ref); // expected-error{{reinterpret_cast to reference type 'int &' changes __capability qualifier}}
  v = static_cast<int &>(cap_ref);           // expected-error{{static_cast to reference type 'int &' changes __capability qualifier}}
  v = const_cast<int &>(cap_ref);            // expected-error{{const_cast to reference type 'int &' changes __capability qualifier}}
  v = dynamic_cast<int &>(cap_ref);          // expected-error{{'int' is not a class}}
  v = (int &)cap_ref;                        // expected-error {{C-style cast to reference type 'int &' changes __capability qualifier}}
  v = intref(cap_ref);                       // expected-error {{functional-style cast to reference type 'intref' (aka 'int &') changes __capability qualifier}}
  v = intref{cap_ref};                       // expected-error {{converting capability type 'int & __capability' to non-capability type 'intref' (aka 'int &') without an explicit cast}}
}

class Foo {
public:
  virtual ~Foo() = default;
};

class Bar : public Foo {
public:
  virtual ~Bar() = default;
};

void cast_classes(Foo &f, Foo &__capability foo_capref) {
  (void)static_cast<Bar &>(f);
  (void)static_cast<Bar & __capability>(f); // expected-error{{static_cast to reference type 'Bar & __capability' changes __capability qualifier}}
  (void)static_cast<Foo &>(foo_capref);     // expected-error{{static_cast to reference type 'Foo &' changes __capability qualifier}}
  (void)static_cast<Bar &>(foo_capref);     // expected-error{{static_cast to reference type 'Bar &' changes __capability qualifier}}

  (void)dynamic_cast<Bar &>(f);
  (void)dynamic_cast<Bar & __capability>(f); // expected-error{{dynamic_cast to reference type 'Bar & __capability' changes __capability qualifier}}
  (void)dynamic_cast<Foo &>(foo_capref);     // expected-error{{dynamic_cast to reference type 'Foo &' changes __capability qualifier}}
  (void)dynamic_cast<Bar &>(foo_capref);     // expected-error{{dynamic_cast to reference type 'Bar &' changes __capability qualifier}}

  (void)const_cast<Bar &>(f);              // expected-error{{const_cast from 'Foo' to 'Bar &' is not allowed}}
  (void)const_cast<Bar & __capability>(f); // expected-error{{const_cast to reference type 'Bar & __capability' changes __capability qualifier}}
  (void)const_cast<Foo &>(foo_capref);     // expected-error{{const_cast to reference type 'Foo &' changes __capability qualifier}}
  (void)const_cast<Bar &>(foo_capref);     // expected-error{{const_cast to reference type 'Bar &' changes __capability qualifier}}

  (void)reinterpret_cast<Bar &>(f);
  (void)reinterpret_cast<Bar & __capability>(f); // expected-error {{reinterpret_cast to reference type 'Bar & __capability' changes __capability qualifier}}
  (void)reinterpret_cast<Foo &>(foo_capref);     // expected-error {{reinterpret_cast to reference type 'Foo &' changes __capability qualifier}}
  (void)reinterpret_cast<Bar &>(foo_capref);     // expected-error {{reinterpret_cast to reference type 'Bar &' changes __capability qualifier}}
}
#endif
