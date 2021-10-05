// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s

int printf(char const *, ...);
int scanf(char const *, ...);

void test(void) {
  printf("%jd", 42.0); // expected-warning {{format specifies type 'intmax_t' (aka 'long')}}
  printf("%ju", 42.0); // expected-warning {{format specifies type 'uintmax_t' (aka 'unsigned long')}}
  printf("%Pd", 42.0); // expected-warning {{format specifies type 'intptr_t' (aka '__intcap')}}
  printf("%Pu", 42.0); // expected-warning {{format specifies type 'uintptr_t' (aka 'unsigned __intcap')}}
  printf("%Px", 42.0); // expected-warning {{format specifies type 'uintptr_t' (aka 'unsigned __intcap')}}
  printf("%PX", 42.0); // expected-warning {{format specifies type 'uintptr_t' (aka 'unsigned __intcap')}}
  printf("%zu", 42.0); // expected-warning {{format specifies type 'size_t' (aka 'unsigned long')}}
  printf("%td", 42.0); // expected-warning {{format specifies type 'ptrdiff_t' (aka 'long')}}
  printf("%lc", 42.0); // expected-warning {{format specifies type 'wint_t' (aka 'int')}}
  printf("%ls", 42.0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}
  printf("%S", 42.0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}
  printf("%C", 42.0);  // expected-warning {{format specifies type 'wchar_t' (aka 'int')}}

  scanf("%jd", 0); // expected-warning {{format specifies type 'intmax_t *' (aka 'long *')}}
  scanf("%ju", 0); // expected-warning {{format specifies type 'uintmax_t *' (aka 'unsigned long *')}}
  scanf("%Pd", 0); // expected-warning {{format specifies type 'intptr_t *' (aka '__intcap *')}}
  scanf("%Pu", 0); // expected-warning {{format specifies type 'uintptr_t *' (aka 'unsigned __intcap *')}}
  scanf("%Px", 0); // expected-warning {{format specifies type 'uintptr_t *' (aka 'unsigned __intcap *')}}
  scanf("%PX", 0); // expected-warning {{format specifies type 'uintptr_t *' (aka 'unsigned __intcap *')}}
  scanf("%zu", 0); // expected-warning {{format specifies type 'size_t *' (aka 'unsigned long *')}}
  scanf("%td", 0); // expected-warning {{format specifies type 'ptrdiff_t *' (aka 'long *')}}
  scanf("%lc", 0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}
  scanf("%ls", 0); // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}
  scanf("%S",  0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}
  scanf("%C",  0);  // expected-warning {{format specifies type 'wchar_t *' (aka 'int *')}}


  // typedef size_t et al. to something crazy.
  typedef void *size_t;
  typedef void *intmax_t;
  typedef void *uintmax_t;
  typedef void *ptrdiff_t;

  // The warning still fires, because it checks the underlying type.
  printf("%jd", (intmax_t)0);  // expected-warning {{format specifies type 'intmax_t' (aka 'long') but the argument has type 'intmax_t __attribute__((cheri_no_provenance))' (aka 'void *')}}
  printf("%ju", (uintmax_t)0); // expected-warning {{format specifies type 'uintmax_t' (aka 'unsigned long') but the argument has type 'uintmax_t __attribute__((cheri_no_provenance))' (aka 'void *')}}
  printf("%zu", (size_t)0);    // expected-warning {{format specifies type 'size_t' (aka 'unsigned long') but the argument has type 'size_t __attribute__((cheri_no_provenance))' (aka 'void *')}}
  printf("%td", (ptrdiff_t)0); // expected-warning {{format specifies type 'ptrdiff_t' (aka 'long') but the argument has type 'ptrdiff_t __attribute__((cheri_no_provenance))' (aka 'void *')}}
}
