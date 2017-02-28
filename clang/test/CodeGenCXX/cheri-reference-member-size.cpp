// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -verify %s -o /dev/null
// expected-no-diagnostics

class A { };

class Foo {
  public:
    A& _a;
    Foo(A& a) : _a(a) { }
};

void f() {
  A a;
  Foo f(a);
}
