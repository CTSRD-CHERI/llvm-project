// RUN: %cheri_cc1 %s -emit-llvm -o - -O2 | FileCheck %s

void callee(int *a);

// CHECK-LABEL: caller
// CHECK: llvm.cheri.cap.address.get
void caller(__uintcap_t len) {
  int ar[len];
  callee(ar);
}
