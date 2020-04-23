// RUN: %cheri_cc1 -fsyntax-only %s -Weverything -Wno-unused -verify
// RUN: %cheri_purecap_cc1 -fsyntax-only %s -Weverything -Wno-unused -verify


typedef void (*fnptr_t)(void);

static void test(const void** cvpp, const void* cvp) {
  fnptr_t f1 = (fnptr_t)cvp; // expected-warning{{cast from 'const void *' to 'void (*)(void)' drops const qualifier}}
  fnptr_t f2 = (fnptr_t)cvpp;
  const void** cvpp1 = (const void**)cvp; // expected-warning{{cast from 'const void *' to 'const void **' drops const qualifier}}
  const void** cvpp2 = (const void**)cvpp;
  const void* cvp1 = (const void*)cvp;
  const void* cvp2 = (const void*)cvpp;
}
