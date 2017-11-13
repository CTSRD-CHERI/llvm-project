// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s

int printf(char const *, ...);
int scanf(char const *, ...);

void test(void) {
  printf("%jd", 42.0); // expected-warning {{format specifies type 'intmax_t' (aka 'long')}}
  printf("%ju", 42.0); // expected-warning {{format specifies type 'uintmax_t' (aka 'unsigned long')}}
  printf("%Pd", 42.0); // expected-warning {{format specifies type 'intptr_t' (aka '__intcap_t')}}
  printf("%Pu", 42.0); // expected-warning {{format specifies type 'uintptr_t' (aka '__uintcap_t')}}
  printf("%zu", 42.0); // expected-warning {{format specifies type 'size_t' (aka 'unsigned long')}}
  printf("%td", 42.0); // expected-warning {{format specifies type 'ptrdiff_t' (aka 'long')}}
  printf("%lc", 42.0); // expected-warning {{format specifies type 'wint_t' (aka 'int')}}
  printf("%ls", 42.0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}
  printf("%S", 42.0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}
  printf("%C", 42.0);  // expected-warning {{format specifies type 'wchar_t' (aka 'int')}}

  scanf("%jd", 0); // expected-warning {{format specifies type 'intmax_t *' (aka 'long * __capability')}}
  scanf("%ju", 0); // expected-warning {{format specifies type 'uintmax_t *' (aka 'unsigned long * __capability')}}
  scanf("%Pd", 0); // expected-warning {{format specifies type 'intptr_t *' (aka '__intcap_t * __capability')}}
  scanf("%Pu", 0); // expected-warning {{format specifies type 'uintptr_t *' (aka '__uintcap_t * __capability')}}
  scanf("%zu", 0); // expected-warning {{format specifies type 'size_t *' (aka 'unsigned long * __capability')}}
  scanf("%td", 0); // expected-warning {{format specifies type 'ptrdiff_t *' (aka 'long * __capability')}}
  scanf("%lc", 0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}
  scanf("%ls", 0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}
  scanf("%S",  0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}
  scanf("%C",  0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int * __capability')}}


  // typedef size_t et al. to something crazy.
  typedef void *size_t;
  typedef void *intmax_t;
  typedef void *uintmax_t;
  typedef void *ptrdiff_t;

  // The warning still fires, because it checks the underlying type.
  printf("%jd", (intmax_t)42); // expected-warning {{format specifies type 'intmax_t' (aka 'long') but the argument has type 'intmax_t' (aka 'void * __capability')}}
  printf("%ju", (uintmax_t)42); // expected-warning {{format specifies type 'uintmax_t' (aka 'unsigned long') but the argument has type 'uintmax_t' (aka 'void * __capability')}}
  printf("%zu", (size_t)42); // expected-warning {{format specifies type 'size_t' (aka 'unsigned long') but the argument has type 'size_t' (aka 'void * __capability')}}
  printf("%td", (ptrdiff_t)42); // expected-warning {{format specifies type 'ptrdiff_t' (aka 'long') but the argument has type 'ptrdiff_t' (aka 'void * __capability')}}
}
