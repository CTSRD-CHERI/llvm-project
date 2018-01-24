// RUN: not --crash %cheri_purecap_cc1 -o - -O0 -emit-llvm %s
// REQUIRES: asserts
// Found after https://github.com/CTSRD-CHERI/cheribsd/commit/58379f87eda5569d6aae0d36154549725b50c1b2
// See https://github.com/CTSRD-CHERI/clang/issues/178
struct a {
  void *ptr;
};
void c(void) {
  struct a first;
  struct a second;
  first.ptr = (__cheri_tocap void *)&second;
}
