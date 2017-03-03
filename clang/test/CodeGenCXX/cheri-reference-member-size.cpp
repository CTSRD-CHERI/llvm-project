// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm \
// RUN:  -fdump-record-layouts -verify %s -o /dev/null
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm \
// RUN:  -fdump-record-layouts %s -o /dev/null 2>&1 | FileCheck %s
// expected-no-diagnostics

// CHECK:     *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class A (empty)
// CHECK-NEXT:    | [sizeof=1, dsize=1, align=1,
// CHECK-NEXT:    |  nvsize=1, nvalign=1]

// CHECK:   LLVMType:%class.A = type { i8 }

// CHECK:     *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class Foo
// CHECK-NEXT:  0 |   class A & _a
// CHECK-NEXT:    | [sizeof=32, dsize=32, align=32,
// CHECK-NEXT:    |  nvsize=32, nvalign=32]

//CHECK: LLVMType:%class.Foo = type { %class.A addrspace(200)* }

// CHECK:      *** Dumping AST Record Layout
// CHECK-NEXT:     0 | class Bar
// CHECK-NEXT:     0 |   class Foo (base)
// CHECK-NEXT:     0 |     class A & _a
// CHECK-NEXT:    32 |   int x
// CHECK-NEXT:       | [sizeof=64, dsize=36, align=32,
// CHECK-NEXT:       |  nvsize=36, nvalign=32]

// CHECK:   LLVMType:%class.Bar = type <{ %class.Foo, i32, [28 x i8] }>

class A { };

class Foo {
  public:
    A& _a;
    Foo(A& a) : _a(a) { }
};

class Bar : public Foo {
  public:
    int x;
    Bar(A& a, int x) : Foo(a), x(x) { }
};

void f() {
  A a;
  Foo f(a);
  Bar b(a, 42);
}
