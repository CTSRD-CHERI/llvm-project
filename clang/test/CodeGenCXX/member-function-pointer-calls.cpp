// RUN: %clang_cc1 %s -triple=x86_64-apple-darwin10 -emit-llvm -O3 -fno-experimental-new-pass-manager  -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple=x86_64-apple-darwin10 -emit-llvm -O3 -fexperimental-new-pass-manager  -o - | FileCheck %s
/// Check that we pass the member pointers indirectly for MinGW64
// RUN: %clang_cc1 %s -triple=x86_64-windows-gnu -emit-llvm -o - | FileCheck %s -check-prefix MINGW64
/// We should be able to optimize calls via the indirectly passed member pointers
// RUN: %clang_cc1 %s -triple=x86_64-windows-gnu -emit-llvm -O3 -fno-experimental-new-pass-manager  -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple=x86_64-windows-gnu -emit-llvm -O3 -fexperimental-new-pass-manager  -o - | FileCheck %s
// For purecap, we currently inline f() but fail to resolve the vtable entry to a direct call.
// RUN: %riscv64_cheri_cc1 %s -emit-llvm -O3 -fno-experimental-new-pass-manager  -o - | FileCheck %s
// RUN: %riscv64_cheri_cc1 %s -emit-llvm -O3 -fexperimental-new-pass-manager  -o - | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -O3 -fno-experimental-new-pass-manager -o - | FileCheck %s --check-prefix=PURECAP
// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -O3 -fexperimental-new-pass-manager -o - | FileCheck %s --check-prefix=PURECAP
struct A {
  virtual int vf1() { return 1; }
  virtual int vf2() { return 2; }
};

int f(A* a, int (A::*fp)()) {
  return (a->*fp)();
}

// CHECK-LABEL: define{{.*}} i32 @_Z2g1v()
// CHECK-NOT: }
// CHECK: ret i32 1
// MINGW64-LABEL: define dso_local i32 @_Z2g1v()
// MINGW64: call i32 @_Z1fP1AMS_FivE(%struct.A* %{{.*}}, { i64, i64 }* %{{.*}})
// PURECAP-LABEL: define{{.*}} i32 @_Z2g1v()
// PURECAP-NOT:    ret i32
// PURECAP:        [[RET:%.*]] = call signext i32 %memptr.virtualfn.i(
// PURECAP:        ret i32 [[RET]]
// PURECAP-NEXT: }
int g1() {
  A a;
  return f(&a, &A::vf1);
}

// CHECK-LABEL: define{{.*}} i32 @_Z2g2v()
// CHECK-NOT: }
// CHECK: ret i32 2
// MINGW64-LABEL: define dso_local i32 @_Z2g2v()
// MINGW64: call i32 @_Z1fP1AMS_FivE(%struct.A* %{{.*}}, { i64, i64 }* %{{.*}})
// PURECAP-LABEL: define{{.*}} i32 @_Z2g2v()
// PURECAP-NEXT: _Z1fP1AMS_FivE.exit:
// PURECAP-NEXT:   ret i32 2
// PURECAP-NEXT: }
int g2() {
  A a;
  return f(&a, &A::vf2);
}
