// RUN: %cheri_cc1 -fsyntax-only -verify=expected,hybrid %s
// RUN: %cheri_cc1 -target-abi purecap -fsyntax-only -verify=expected,purecap %s

// The "pointer" machine mode is always valid on pointer types and yields the
// same underlying type. NB: Unlike GCC, Clang does not allow the attribute to
// be used on language-level pointers, only integer and floating-point types
// (and, for CHERI, integer capability types).

typedef __INTPTR_TYPE__ intptr_ptr __attribute__((mode(pointer)));
typedef __UINTPTR_TYPE__ uintptr_ptr __attribute__((mode(pointer)));

_Static_assert(__builtin_types_compatible_p(intptr_ptr, __INTPTR_TYPE__), "");
_Static_assert(__builtin_types_compatible_p(uintptr_ptr, __UINTPTR_TYPE__), "");

// Similarly, "capability" is always valid on capability types and yields the
// same underlying type.

typedef __intcap_t intcap_cap __attribute__((mode(capability)));
typedef __uintcap_t uintcap_cap __attribute__((mode(capability)));

_Static_assert(__builtin_types_compatible_p(intcap_cap, __intcap_t), "");
_Static_assert(__builtin_types_compatible_p(uintcap_cap, __uintcap_t), "");

// Attempting to use the "pointer" mode on an integer is only an error for
// pure-capability code.

typedef signed signed_ptr __attribute__((mode(pointer))); // purecap-error{{type of machine mode does not match type of base type}}
typedef unsigned unsigned_ptr __attribute__((mode(pointer))); // purecap-error{{type of machine mode does not match type of base type}}

// Attempting to use the "capability" mode on an integer is always an error.

typedef signed signed_cap __attribute__((mode(capability))); // expected-error{{type of machine mode does not match type of base type}}
typedef unsigned unsigned_cap __attribute__((mode(capability))); // expected-error{{type of machine mode does not match type of base type}}

// Attempting to use an integer mode (in this case we use DI, the double
// integer, i.e. 64-bit, mode since it corresponds to the address size and is
// most likely to accidentally work) on a capability is always an error.

typedef __intcap_t intcap_int __attribute__((mode(DI))); // expected-error{{type of machine mode does not match type of base type}}
typedef __uintcap_t uintcap_int __attribute__((mode(DI))); // expected-error{{type of machine mode does not match type of base type}}

// The "unwind_word" machine mode is always the same as "pointer".

typedef __INTPTR_TYPE__ intptr_unw __attribute__((mode(unwind_word)));
typedef __UINTPTR_TYPE__ uintptr_unw __attribute__((mode(unwind_word)));

_Static_assert(__builtin_types_compatible_p(intptr_unw, __INTPTR_TYPE__), "");
_Static_assert(__builtin_types_compatible_p(uintptr_unw, __UINTPTR_TYPE__), "");

// The "word" machine mode is always an integer of the same width as a pointer
// address, which for CHERI is always the same underlying type as ptrdiff_t
// (signed) or ptraddr_t (unsigned).

typedef signed signed_word __attribute__((mode(word)));
typedef unsigned unsigned_word __attribute__((mode(word)));

_Static_assert(__builtin_types_compatible_p(signed_word, __PTRDIFF_TYPE__), "");
_Static_assert(__builtin_types_compatible_p(unsigned_word, __PTRADDR_TYPE__), "");
