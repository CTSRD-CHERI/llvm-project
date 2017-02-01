// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox  %s -O0 -msoft-float -S -emit-llvm -o - | FileCheck %s
// Ensure literals don't spontaneously switch address space during calls when
// using the sandbox ABI. A regression test for #5.

char takes_string_ptr(const char *a) {
    return a[0];
}

// CHECK-LABEL: main
int main(void) {
  // CHECK: call signext i8 %{{.*}}(i8 addrspace(200)*
  return takes_string_ptr("hi");
}
