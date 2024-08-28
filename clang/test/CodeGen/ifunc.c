// RUN: %clang_cc1 -triple i386-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i386-unknown-linux-gnu -O2 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple avr-unknown-unknown -emit-llvm -o - %s | FileCheck %s --check-prefix=AVR

int foo(int) __attribute__ ((ifunc("foo_ifunc")));

static int f1(int i) {
  return i + 1;
}

static int f2(int i) {
  return i + 2;
}

typedef int (*foo_t)(int);

int global;

static foo_t foo_ifunc(void) {
  return global ? f1 : f2;
}

int bar(void) {
  return foo(1);
}

extern void goo(void);

void bar2(void) {
  goo();
}

extern void goo(void) __attribute__ ((ifunc("goo_ifunc")));

void* goo_ifunc(void) {
  return 0;
}
// CHECK: @foo = ifunc i32 (i32), ptr @foo_ifunc
// CHECK: @goo = ifunc void (), ptr @goo_ifunc

// AVR: @foo = ifunc i16 (i16), ptr addrspace(1) @foo_ifunc
// AVR: @goo = ifunc void (), ptr addrspace(1) @goo_ifunc

// CHECK: call i32 @foo(i32
// CHECK: call void @goo()
