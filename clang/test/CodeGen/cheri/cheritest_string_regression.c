// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck %s
// Ensure that the code below doesn't crash due to the __cheri_tocap to void*
// Found after https://github.com/CTSRD-CHERI/cheribsd/commit/58379f87eda5569d6aae0d36154549725b50c1b2
// See https://github.com/CTSRD-CHERI/clang/issues/178
struct a {
  void *ptr;
};
void c(void) {
  struct a first;
  struct a second;
  // CHECK: bitcast %struct.a addrspace(200)* %second to i8 addrspace(200)*
  first.ptr = (__cheri_tocap void *)&second;
}
