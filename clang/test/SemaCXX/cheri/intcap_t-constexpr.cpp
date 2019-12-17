/// Check that we can handle __intcap_t in C++ constexpr functions
/// Also check that we get the same errors as we would when using long
// RUN: %cheri_cc1 -DSIGNED=1 -fsyntax-only %s -verify=expected,hybrid,signed-long "-DINTTYPE=long" -Wpedantic
// RUN: %cheri_cc1 -DSIGNED=0 -fsyntax-only %s -verify=expected,hybrid,unsigned-long "-DINTTYPE=unsigned long" -Wpedantic
// RUN: %cheri_cc1 -DSIGNED=1 -fsyntax-only %s -verify=expected,hybrid,signed-intcap -DINTTYPE=__intcap_t -Wpedantic
// RUN: %cheri_cc1 -DSIGNED=0 -fsyntax-only %s -verify=expected,hybrid,unsigned-intcap -DINTTYPE=__uintcap_t -Wpedantic
// RUN: %cheri_purecap_cc1 -DSIGNED=1 -fsyntax-only %s -verify=expected,purecap,signed-intcap -DINTTYPE=__intcap_t -Wpedantic
// RUN: %cheri_purecap_cc1 -DSIGNED=0 -fsyntax-only %s -verify=expected,purecap,unsigned-intcap -DINTTYPE=__uintcap_t -Wpedantic

// Basic addition should work:
// constexpr __intcap_t x1 = __intcap_t{3} + __intcap_t{1};
// static_assert(x1 == 4, "");
// Check that it also works if we wrap the addition in a constexpr function

// First sanity-check with long
// constexpr long add_long(long a, long b) { return a + b; }
constexpr long l1 = 1;
static_assert(l1 + 2 == 3, "");
constexpr INTTYPE ic1 = 1;
static_assert(ic1 + 2 == 3, "");
static_assert(ic1 + 2 == static_cast<INTTYPE>(3), "");
static_assert(ic1 + static_cast<INTTYPE>(2) == static_cast<INTTYPE>(3), "");

constexpr INTTYPE add_mixed(long a, INTTYPE b) { return a + b; }
constexpr INTTYPE x2 = add_mixed(3, 10);
static_assert(x2 == 13, "");
static_assert(add_mixed(3, 11) == 14, "");

constexpr INTTYPE add_intcap(INTTYPE a, INTTYPE b) { return a + b; }
constexpr INTTYPE x3 = add_intcap(7, 8);
static_assert(x3 == 15, "");

// But not the the other way around: casts from pointers are not handled
int x;
static_assert(&x == &x, "");
static_assert(&x == (int*)(__INTPTR_TYPE__)&x, ""); // expected-error{{not an integral constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert(&x == (int*)(INTTYPE)&x, ""); // expected-error{{not an integral constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert((((__INTPTR_TYPE__)&x) & 3) == 0, ""); // expected-error{{not an integral constant expression}}
static_assert((((INTTYPE)&x) & 3) == 0, ""); // expected-error{{not an integral constant expression}}

// Check that we can look through casts of INTTYPE -> pointer
constexpr void* voidptr_constant = (void*)0x1; // expected-error{{constexpr variable 'voidptr_constant' must be initialized by a constant expression}}
// expected-note@-1{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
constexpr long long_from_voidptr_constant = (long)(void*)0x1;  // expected-note{{declared here}}
// expected-error@-1{{constexpr variable 'long_from_voidptr_constant' must be initialized by a constant expression}}
// expected-note@-2{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
// purecap-warning@-3{{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
static_assert(long_from_voidptr_constant == 1, ""); // expected-error{{static_assert expression is not an integral constant expression}}
// expected-note@-1{{initializer of 'long_from_voidptr_constant' is not a constant expression}}
constexpr INTTYPE intcap_from_voidptr_constant = (INTTYPE)(void*)0x2;  // expected-note{{declared here}}
// expected-error@-1{{constexpr variable 'intcap_from_voidptr_constant' must be initialized by a constant expression}}
// expected-note@-2{{cast that performs the conversions of a reinterpret_cast is not allowed in a constant expression}}
static_assert(intcap_from_voidptr_constant == 2, ""); // expected-error{{static_assert expression is not an integral constant expression}}
// expected-note@-1{{initializer of 'intcap_from_voidptr_constant' is not a constant expression}}


// Check overflow
#define __INT64_MIN__ (-__INT64_MAX__ - 1)
constexpr INTTYPE check_overflow0 = static_cast<INTTYPE>(__INT64_MAX__);
constexpr INTTYPE check_overflow1 = static_cast<INTTYPE>(__INT64_MAX__) + 1;
// signed-long-error@-1{{must be initialized by a constant expression}}
// signed-long-note@-2{{value 9223372036854775808 is outside the range of representable values of type 'long'}}
// signed-intcap-error@-3{{must be initialized by a constant expression}}
// signed-intcap-note@-4{{value 9223372036854775808 is outside the range of representable values of type '__intcap_t'}}
constexpr INTTYPE check_overflow2 = static_cast<INTTYPE>(__INT64_MIN__);
constexpr INTTYPE check_overflow3 = static_cast<INTTYPE>(__INT64_MIN__) -1;
// signed-long-error@-1{{must be initialized by a constant expression}}
// signed-long-note@-2{{value -9223372036854775809 is outside the range of representable values of type 'long'}}
// signed-intcap-error@-3{{must be initialized by a constant expression}}
// signed-intcap-note@-4{{value -9223372036854775809 is outside the range of representable values of type '__intcap_t'}}

// No warnings for unsigned since it wraps around
constexpr INTTYPE check_overflow4 = static_cast<__UINT64_TYPE__>(0);
constexpr INTTYPE check_overflow5 = static_cast<__UINT64_TYPE__>(0) - 1;
#if SIGNED
static_assert(check_overflow5 == -1, "");
#else
static_assert(check_overflow5 == __UINT64_MAX__, "");
#endif
constexpr INTTYPE check_overflow6 = static_cast<INTTYPE>(__UINT64_MAX__);
#if SIGNED
static_assert(check_overflow6 == -1, "");
#else
static_assert(check_overflow6 == __UINT64_MAX__, "");
#endif
constexpr INTTYPE check_overflow7 = static_cast<INTTYPE>(__UINT64_MAX__) + 1;
static_assert(check_overflow7 == 0, "");
