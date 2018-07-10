// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s

typedef unsigned long size_t;

const char * collatenames[] = { "A" };

template <size_t _Np>
const char** begin(const char * (&__array)[_Np]) {
  // CHECK: FunctionDecl {{.*}} {{.*}} {{.*}} used begin 'const char * __capability * __capability (const char * __capability (& __capability)[1])'
  return __array;
}

int main(void) {
  begin(collatenames);
  return 0;
}
