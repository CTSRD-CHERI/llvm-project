// RUN: %cheri128_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-HYBRID-128 %s
// RUN: %cheri128_cc1 -target-abi purecap -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-PURECAP-128 %s
// RUN: %cheri256_cc1 -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-HYBRID-256 %s
// RUN: %cheri256_cc1 -target-abi purecap -emit-llvm -o - %s \
// RUN:   | FileCheck -check-prefix=CHECK-PURECAP-256 %s

int check_pointer() {
  return sizeof(int *);
// CHECK-HYBRID-128: ret i32 8
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-256: ret i32 8
// CHECK-PURECAP-256: ret i32 32
}

int check_capability_pointer() {
  return sizeof(int *__capability);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-256: ret i32 32
// CHECK-PURECAP-256: ret i32 32
}

int check_capability_reference() {
  struct A {
    int &__capability v;
  };
  return sizeof(A);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-256: ret i32 32
// CHECK-PURECAP-256: ret i32 32
}

int check_capability_rvalue_reference() {
  struct A {
    int &&__capability v;
  };
  return sizeof(A);
// CHECK-HYBRID-128: ret i32 16
// CHECK-PURECAP-128: ret i32 16
// CHECK-HYBRID-256: ret i32 32
// CHECK-PURECAP-256: ret i32 32
}
