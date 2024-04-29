// RUN: %cheri_purecap_cc1 -emit-llvm \
// RUN:  -fdump-record-layouts -verify %s -o /dev/null 2>&1 | FileCheck %s
// expected-no-diagnostics

// CHECK-LABEL: *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class Foo
// CHECK-NEXT:  0 |   A & __capability _a
// CHECK-NEXT:    | [sizeof=16,
// CHECK-SAME:       dsize=16, align=16,
// CHECK-NEXT:    |  nvsize=16, nvalign=16]

// CHECK-LABEL: *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class A (empty)
// CHECK-NEXT:    | [sizeof=1, dsize=1, align=1,
// CHECK-NEXT:    |  nvsize=1, nvalign=1]

// CHECK-LABEL:    *** Dumping AST Record Layout
// CHECK-NEXT:     0 | class Bar
// CHECK-NEXT:     0 |   class Foo (base)
// CHECK-NEXT:     0 |     A & __capability _a
// CHECK-NEXT:    16 |   int x
// CHECK-NEXT:       | [sizeof=32, dsize=20, align=16,
// CHECK-NEXT:       |  nvsize=20, nvalign=16]

// CHECK-LABEL: *** Dumping IRgen Record Layout
// CHECK: LLVMType:%class.A = type { i8 }
// CHECK: LLVMType:%class.Foo = type { ptr addrspace(200) }
// CHECK: LLVMType:%class.Bar = type <{ %class.Foo, i32, [12 x i8] }>

class A {};

class Foo {
public:
  A &_a;
  Foo(A &a) : _a(a) {}
};

class Bar : public Foo {
public:
  int x;
  Bar(A &a, int x) : Foo(a), x(x) {}
};

void f() {
  A a;
  Foo f(a);
  Bar b(a, 42);
}
