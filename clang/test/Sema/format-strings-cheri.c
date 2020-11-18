// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify=expected,hybrid %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -verify=expected,purecap %s

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
  printf("%lp", &p); // hybrid-warning{{format specifies type 'void * __capability' but the argument has type 'void * __capability *'}}
  printf("%lp", (void *)0); // hybrid-warning{{format specifies type 'void * __capability' but the argument has type 'void *'}}
  printf("%lp", (void **)(void *)0); // hybrid-warning{{format specifies type 'void * __capability' but the argument has type 'void **'}}
  printf("%lp", (int *)(void *)0); // hybrid-warning{{format specifies type 'void * __capability' but the argument has type 'int *'}}
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
