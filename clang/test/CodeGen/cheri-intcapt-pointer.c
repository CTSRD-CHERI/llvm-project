// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s

int main(void) {
  __intcap_t *t;
  // CHECK: {{%.+}} = load i8 addrspace(200)**, i8 addrspace(200)*** %t
  if (t) {
  }
  return 0;
}
