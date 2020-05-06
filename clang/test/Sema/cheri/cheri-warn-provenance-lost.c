// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify=purecap
// RUN: %cheri_cc1 -fsyntax-only %s -verify=hybrid
// hybrid-no-diagnostics

void *__capability a(long b) {
  return (void *__capability)b; // purecap-warning {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
}
void *__capability a_fixed(long b) {
  return (void *__capability)(__intcap_t)b;
}
void *__capability c(__intcap_t b) {
  return (void *__capability)b;
}
