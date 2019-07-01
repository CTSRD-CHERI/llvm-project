// RUN: %cheri_cc1 -ast-dump -o /dev/null %s 2>&1 | FileCheck %s -check-prefix AST
// RUN: %cheri_cc1 -emit-llvm -o - %s | %cheri_FileCheck %s

// Check that a UncheckedDerivedToBase cast does not remove the __capability qualifier

// AST:      `-FunctionDecl {{.+}} func 'const long & __capability ()'
// AST-NEXT:  `-CompoundStmt
// AST-NEXT:   `-ReturnStmt
// AST-NEXT:    `-MemberExpr {{.+}} 'const long' lvalue ->size
// AST-NEXT:     `-ImplicitCastExpr {{.+}} 's1 * __capability' <UncheckedDerivedToBase (s1)>
// AST-NEXT:      `-ImplicitCastExpr {{.+}} 'const s2 * __capability' <LValueToRValue>
// AST-NEXT:       `-DeclRefExpr {{.+}} 'const s2 * __capability' lvalue Var {{.+}} 'c_s2' 'const s2 * __capability'

struct s1 {
  long size;
};

struct s2 : public s1 {
};

const s2 *__capability c_s2;

// CHECK-LABEL: _Z4funcv
// CHECK: load %struct.s2 addrspace(200)*, %struct.s2 addrspace(200)** @c_s2, align [[#CAP_SIZE]]
const long &__capability func() {
  return c_s2->size;
}
