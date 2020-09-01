// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -verify %s

int printf(const char *restrict, ...);

void test(void *p) {
  printf("%#p", p);
  printf("%.4p", p); // expected-warning{{precision used with 'p' conversion specifier, resulting in undefined behavior}}
  printf("%#.4p", p);
}
