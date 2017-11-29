// RUN: %cheri_purecap_cc1 -emit-llvm \
// RUN:  -fdump-record-layouts -verify %s -o /dev/null
// RUN: %cheri_purecap_cc1 -emit-llvm \
// RUN:  -fdump-record-layouts %s -o /dev/null 2>&1 | %cheri_FileCheck %s
// expected-no-diagnostics

// CHECK:     *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class A (empty)
// CHECK-NEXT:    | [sizeof=1, dsize=1, align=1,
// CHECK-NEXT:    |  nvsize=1, nvalign=1]

// CHECK:   LLVMType:%class.A = type { i8 }

// CHECK:     *** Dumping AST Record Layout
// CHECK-NEXT:  0 | class Foo
// CHECK-NEXT:  0 |   class A & __capability _a
// CHECK-NEXT:    | [sizeof=[[$CAP_SIZE]],
// CHECK-SAME:       dsize=[[$CAP_SIZE]], align=[[$CAP_SIZE]],
// CHECK-NEXT:    |  nvsize=[[$CAP_SIZE]], nvalign=[[$CAP_SIZE]]]

//CHECK: LLVMType:%class.Foo = type { %class.A addrspace(200)* }

// CHECK:      *** Dumping AST Record Layout
// CHECK-NEXT:     0 | class Bar
// CHECK-NEXT:     0 |   class Foo (base)
// CHECK-NEXT:     0 |     class A & __capability _a
// CHECK-NEXT:    [[$CAP_SIZE]] |   int x
// CHECK-NEXT:       | [sizeof=[[@EXPR $CAP_SIZE * 2]], dsize=[[@EXPR $CAP_SIZE + 4]], align=[[$CAP_SIZE]],
// CHECK-NEXT:       |  nvsize=[[@EXPR $CAP_SIZE + 4]], nvalign=[[$CAP_SIZE]]]

// CHECK:   LLVMType:%class.Bar = type <{ %class.Foo, i32, {{\[}}[[@EXPR $CAP_SIZE - 4]] x i8] }>

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
