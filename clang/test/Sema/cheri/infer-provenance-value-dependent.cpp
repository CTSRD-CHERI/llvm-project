// RUN: %cheri_purecap_cc1 -o - -fsyntax-only  %s -verify -Weverything -Wno-missing-prototypes
// expected-no-diagnostics

template<int c> void* b() {
  return reinterpret_cast<void *>(c);
}

void* test() {
  // instantiate:
  return b<123>();
}
