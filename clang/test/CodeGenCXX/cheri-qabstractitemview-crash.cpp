// RUN: %clang_cc1 -triple x86_64-unknown-freebsd -o - -emit-llvm %s
// RUN: %cheri_cc1 -target-abi purecap -o - -emit-llvm %s
int a(const int &i = int()) {
  return i;
}

bool b(void) {
  return a() == a();
}
