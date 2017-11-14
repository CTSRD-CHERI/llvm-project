// RUN: %cheri_purecap_cc1 -std=gnu99 -ftls-model=local-exec -o - -O0 -emit-llvm  %s
// calling a weak function used to crash the compiler

void a(void);
#pragma weak a
void b(void) {
  a();
}
