// RUN: %clang %s -mabi=sandbox -cheri-linker -fno-rtti -std=c++11 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s
// RUN: %clang %s -mabi=sandbox -cheri-linker -fno-rtti -std=c++11 -target cheri-unknown-freebsd -o - -emit-llvm -S -O2 | FileCheck %s -check-prefix CHECK-OPT
// RUN: %clang %s -mabi=sandbox -cheri-linker -fno-rtti -std=c++11 -target cheri-unknown-freebsd -o - -emit-llvm -S -O3 | FileCheck %s -check-prefix CHECK-OPT

// FIXME what should member pointer layout be on CHERI?
// CHECK-NOT: %data_ptr = alloca i64, align 8
// CHECK-NOT: %func_ptr = alloca { i64, i64 }, align 8
// CHECK-NOT: %virtual_func_ptr = alloca { i64, i64 }, align 8
class A {
public:
  int x = 3;
  int foo() { return 1; }
  virtual int foo_virtual() { return 2; }
};

int main() {
  A a;
  int A::* data_ptr = &A::x;
  int (A::* func_ptr)()  = &A::foo;
  int (A::* virtual_func_ptr)() = &A::foo_virtual;
  return a.*data_ptr + (a.*func_ptr)() + (a.*virtual_func_ptr)();
}
