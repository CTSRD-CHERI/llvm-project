// RUN: %cheri_cc1 -fsyntax-only %s -target-abi purecap
// RUN: %cheri_cc1 -fsyntax-only %s

typedef void(*ptr)(void);

void bar(void);

ptr x = bar;

_Static_assert(sizeof(x) == sizeof(void*), "Size of function pointer is incorrect");
_Static_assert(sizeof(ptr) == sizeof(void*), "Size of function pointer is incorrect");
_Static_assert(sizeof(const ptr) == sizeof(void*), "Size of function pointer is incorrect");
_Static_assert(sizeof(&bar) == sizeof(void*), "Size of function pointer is incorrect");

