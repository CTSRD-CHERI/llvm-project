// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap %s -std=c++14 -emit-llvm -o -
// XFAIL: *
// Bitfields with underlying type __uintcap_t used to cause assertions. Found while compiling qdatetime.cpp for purecap ABI

typedef __intcap_t a;
class b {
  void c();
  struct B {
    a e : sizeof(void *);
  };
  struct {
    B data;
  } d;
};
long f;
void b::c() { d.data.e = f; }
