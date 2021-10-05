// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify=expected,hybrid %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -verify=expected,purecap %s
/// There should not be any additional warnings when compiling with -Wformat-pedantic
/// (at least until Clang implements -Wformat-signedness).
// RUN: %riscv64_cheri_cc1 -fsyntax-only -Wformat-pedantic -verify=expected,hybrid,pedantic,hybrid-pedantic %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -Wformat-pedantic -verify=expected,purecap,pedantic,purecap-pedantic %s

int printf(const char *restrict, ...);

void test_plain_pointer(void *p) {
  printf("%#p", p);
  printf("%.4p", p); // expected-warning{{precision used with 'p' conversion specifier, resulting in undefined behavior}}
  printf("%#.4p", p);
}

void test_long_pointer(void * __capability p) {
  printf("%lp", p);
  printf("%#lp", p);
  printf("%.4lp", p); // expected-warning{{precision used with 'p' conversion specifier, resulting in undefined behavior}}
  printf("%#.4lp", p);
  printf("%lp", &p);
  // hybrid-warning@-1{{format specifies type 'void * __capability' but the argument has type 'void * __capability *'}}
  // purecap-pedantic-warning@-2{{format specifies type 'void *' but the argument has type 'void **'}}
  printf("%lp", (void *)0);
  // hybrid-warning@-1{{format specifies type 'void * __capability' but the argument has type 'void *'}}
  printf("%lp", (void **)(void *)0);
  // hybrid-warning@-1{{format specifies type 'void * __capability' but the argument has type 'void **'}}
  // purecap-pedantic-warning@-2{{format specifies type 'void *' but the argument has type 'void ** __attribute__((cheri_no_provenance))'}}
  printf("%lp", (int *)(void *)0);
  // hybrid-warning@-1{{format specifies type 'void * __capability' but the argument has type 'int *'}}
  // purecap-pedantic-warning@-2{{format specifies type 'void *' but the argument has type 'int * __attribute__((cheri_no_provenance))'}}
}

void test_invalid_length_modifiers(void *p) {
  printf("%hhp", p); // expected-warning{{length modifier 'hh' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%hp", p); // expected-warning{{length modifier 'h' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%llp", p); // expected-warning{{length modifier 'll' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%jp", p); // expected-warning{{length modifier 'j' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%tp", p); // expected-warning{{length modifier 't' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%zp", p); // expected-warning{{length modifier 'z' results in undefined behavior or no effect with 'p' conversion specifier}}
  printf("%qp", p); // expected-warning{{length modifier 'q' results in undefined behavior or no effect with 'p' conversion specifier}}
}

#ifdef __CHERI_PURE_CAPABILITY__
#define PRIxPTR "Px"
#define PRIdPTR "Pd"
#else
#define PRIxPTR "lx"
#define PRIdPTR "ld"
#endif
void test_priptr(__PTRDIFF_TYPE__ saddr, __PTRADDR_TYPE__ uaddr, __INTPTR_TYPE__ sptr, __UINTPTR_TYPE__ uptr) {
  // The x format string expects an unsigned type
  // FIXME: However, we should no warn about this until Clang implements -Wformat-signedness
  printf("%" PRIxPTR, saddr); // purecap-warning{{format specifies type 'uintptr_t' (aka 'unsigned __intcap') but the argument has type 'long'}}
  printf("%" PRIxPTR, uaddr); // purecap-warning{{format specifies type 'uintptr_t' (aka 'unsigned __intcap') but the argument has type 'unsigned long'}}
  printf("%" PRIxPTR, sptr);  // purecap-warning{{format specifies type 'uintptr_t' (aka 'unsigned __intcap') but the argument has type '__intcap'}}
  printf("%" PRIxPTR, uptr);
  printf("%" PRIxPTR, (__INTPTR_TYPE__)saddr); // purecap-warning{{format specifies type 'uintptr_t' (aka 'unsigned __intcap') but the argument has type '__intcap __attribute__((cheri_no_provenance))'}}
  printf("%" PRIxPTR, (__UINTPTR_TYPE__)uaddr);

  // And the d format string expects a signed one:
  // FIXME: However, we should no warn about this until Clang implements -Wformat-signedness
  printf("%" PRIdPTR, saddr); // purecap-warning{{format specifies type 'intptr_t' (aka '__intcap') but the argument has type 'long'}}
  printf("%" PRIdPTR, uaddr); // purecap-warning{{format specifies type 'intptr_t' (aka '__intcap') but the argument has type 'unsigned long'}}
  printf("%" PRIdPTR, sptr);
  printf("%" PRIdPTR, uptr); // purecap-warning{{format specifies type 'intptr_t' (aka '__intcap') but the argument has type 'unsigned __intcap'}}
  printf("%" PRIdPTR, (__INTPTR_TYPE__)saddr);
  printf("%" PRIdPTR, (__UINTPTR_TYPE__)uaddr); // purecap-warning{{format specifies type 'intptr_t' (aka '__intcap') but the argument has type 'unsigned __intcap __attribute__((cheri_no_provenance))'}}
}
