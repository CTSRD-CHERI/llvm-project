// RUN: %cheri_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-HYBRID-128 %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-PURECAP-128 %s
// RUN: %riscv64_cheri_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-HYBRID-128 %s
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-PURECAP-128 %s
// RUN: %riscv32_cheri_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-HYBRID-64 %s
// RUN: %riscv32_cheri_purecap_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-PURECAP-64 %s

int check_pointer() {
  return sizeof(int *);
// CHECK-HYBRID-128: ret i32 8
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-64: ret i32 4
// CHECK-PURECAP-64: ret i32 8
}

int check_capability_pointer() {
  return sizeof(int *__capability);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-64: ret i32 8
// CHECK-PURECAP-64: ret i32 8
}

int check_capability_reference() {
  struct A {
    int &__capability v;
  };
  return sizeof(A);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-64: ret i32 8
// CHECK-PURECAP-64: ret i32 8
}

int check_capability_rvalue_reference() {
  struct A {
    int &&__capability v;
  };
  return sizeof(A);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-64: ret i32 8
// CHECK-PURECAP-64: ret i32 8
}
