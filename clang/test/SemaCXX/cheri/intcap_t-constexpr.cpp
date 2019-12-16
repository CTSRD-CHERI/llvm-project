/// Check that we can handle __intcap_t in C++ constexpr functions
// RUN: %cheri_cc1 -fsyntax-only %s -verify -Wpedantic
// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify=expected,purecap -Wpedantic

// Basic addition should work:
// constexpr __intcap_t x1 = __intcap_t{3} + __intcap_t{1};
// static_assert(x1 == 4, "");
// Check that it also works if we wrap the addition in a constexpr function

// First sanity-check with long
// constexpr long add_long(long a, long b) { return a + b; }
constexpr long l1 = 1;
static_assert(l1 + 2 == 3, "");
constexpr __intcap_t ic1 = 1;
static_assert(ic1 + 2 == 3, "");
static_assert(ic1 + 2 == __intcap_t{3}, "");
static_assert(ic1 + __intcap_t{2} == __intcap_t{3}, "");

constexpr __intcap_t add_mixed(long a, __intcap_t b) { return a + b; }
constexpr __intcap_t x2 = add_mixed(3, 10);
static_assert(x2 == 13, "");
static_assert(add_mixed(3, 11) == 14, "");

constexpr __intcap_t add_intcap(__intcap_t a, __intcap_t b) { return a + b; }
constexpr __intcap_t x3 = add_intcap(7, 8);
static_assert(x3 == 15, "");

// But not the the other way around: casts from pointers are not handled
int x;
static_assert(&x == &x, "");
static_assert(&x == (int*)(__INTPTR_TYPE__)&x, ""); // expected-error{{not an integral constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert(&x == (int*)(__intcap_t)&x, ""); // expected-error{{not an integral constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert((((__INTPTR_TYPE__)&x) & 3) == 0, ""); // expected-error{{not an integral constant expression}}
static_assert((((__intcap_t)&x) & 3) == 0, ""); // expected-error{{not an integral constant expression}}

// Check that we can look through casts of __intcap_t -> pointer
constexpr void* voidptr_constant = (void*)0x1; // expected-error{{constexpr variable 'voidptr_constant' must be initialized by a constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
constexpr long long_from_voidptr_constant = (long)(void*)0x1;  // expected-note{{declared here}}
// expected-error@-1{{constexpr variable 'long_from_voidptr_constant' must be initialized by a constant expression}}
// expected-note@-2{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
// purecap-warning@-3{{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
static_assert(long_from_voidptr_constant == 1, ""); // expected-error{{static_assert expression is not an integral constant expression}}
// expected-note@-1{{initializer of 'long_from_voidptr_constant' is not a constant expression}}
constexpr __intcap_t intcap_from_voidptr_constant = (__intcap_t)(void*)0x2;  // expected-note{{declared here}}
// expected-error@-1{{constexpr variable 'intcap_from_voidptr_constant' must be initialized by a constant expression}}
// expected-note@-2{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert(intcap_from_voidptr_constant == 2, ""); // expected-error{{static_assert expression is not an integral constant expression}}
// expected-note@-1{{initializer of 'intcap_from_voidptr_constant' is not a constant expression}}
