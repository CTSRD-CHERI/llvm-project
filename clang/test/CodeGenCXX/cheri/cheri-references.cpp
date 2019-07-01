// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | %cheri_FileCheck %s

class A { };

void f() {
  A a1;
  // CHECK: %a1 = alloca %class.A, align 1
  A& a2 = a1;
  // CHECK-NEXT: %a2 = alloca %class.A addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK-NEXT: %b = alloca i8, align 1
  // CHECK-NEXT: %b2 = alloca i8, align 1
  // CHECK-NEXT: store %class.A addrspace(200)* %a1, %class.A addrspace(200)* addrspace(200)* %a2, align [[#CAP_SIZE]]
  bool b = &a2 == (A*)0x1234567;
  // CHECK-NEXT: [[REF_ADDR:%.+]] = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a2, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[FAKE_CAP:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 19088743)
  // CHECK-NEXT: [[FAKE_A_PTR:%.+]] = bitcast i8 addrspace(200)* [[FAKE_CAP]] to %class.A addrspace(200)*
  // CHECK-NEXT: %cmp = icmp eq %class.A addrspace(200)* [[REF_ADDR]], [[FAKE_A_PTR]]
  // CHECK-NEXT: %frombool = zext i1 %cmp to i8
  // CHECK-NEXT: store i8 %frombool, i8 addrspace(200)* %b, align 1
  bool b2 = &a2 == (void*)0x1234567;
  // CHECK-NEXT: [[REF_ADDR:%.+]] = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a2, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[REF_ADDR_VOID:%.+]] = bitcast %class.A addrspace(200)* [[REF_ADDR]] to i8 addrspace(200)*
  // CHECK-NEXT: [[FAKE_VOID_PTR:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 19088743)
  // CHECK-NEXT: %cmp1 = icmp eq i8 addrspace(200)* [[REF_ADDR_VOID]], [[FAKE_VOID_PTR]]
  // CHECK-NEXT: %frombool2 = zext i1 %cmp1 to i8
  // CHECK-NEXT: store i8 %frombool2, i8 addrspace(200)* %b2, align 1

  // AST for the two:
#if 0
    |-DeclStmt 0x1f8c600 <line:11:3, col:32>
    | `-VarDecl 0x1f8c3e8 <col:3, col:23> col:8 b '_Bool' cinit
    |   `-BinaryOperator 0x1f8c580 <col:12, col:23> '_Bool' '=='
    |     |-ImplicitCastExpr 0x1f8c550 <col:12, col:13> 'class A *' <NoOp>
    |     | `-UnaryOperator 0x1f8c4a0 <col:12, col:13> 'class A * __capability' prefix '&'
    |     |   `-DeclRefExpr 0x1f8c448 <col:13> 'class A' lvalue Var 0x1f8c338 'a2' 'class A &'
    |     `-ImplicitCastExpr 0x1f8c568 <col:19, col:23> 'class A *' <NoOp>
    |       `-CStyleCastExpr 0x1f8c4f0 <col:19, col:23> 'class A * __capability' <IntegralToPointer>
    |         `-IntegerLiteral 0x1f8c4c0 <col:23> 'int' 19088743
    `-DeclStmt 0x1f8c7b8 <line:13:3, col:36>
      `-VarDecl 0x1f8c628 <col:3, col:27> col:8 b2 '_Bool' cinit
        `-BinaryOperator 0x1f8c790 <col:13, col:27> '_Bool' '=='
          |-ImplicitCastExpr 0x1f8c760 <col:13, col:14> 'void *' <BitCast>
          | `-UnaryOperator 0x1f8c6b0 <col:13, col:14> 'class A * __capability' prefix '&'
          |   `-DeclRefExpr 0x1f8c688 <col:14> 'class A' lvalue Var 0x1f8c338 'a2' 'class A &'
          `-ImplicitCastExpr 0x1f8c778 <col:20, col:27> 'void *' <NoOp>
            `-CStyleCastExpr 0x1f8c708 <col:20, col:27> 'void * __capability' <IntegralToPointer>
              `-IntegerLiteral 0x1f8c6d0 <col:27> 'int' 19088743

#endif
}
