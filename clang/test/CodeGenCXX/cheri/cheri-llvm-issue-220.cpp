// Check that the source from https://github.com/CTSRD-CHERI/llvm/issues/220 doesn't crash:
// REQUIRES: mips-registered-target
// FIXME: this is a terrible test and should either be deleted or converted to an LLC test...

// RUN: %clang_cc1 -triple mips64-unknown-freebsd -S -disable-free -disable-llvm-verifier -discard-value-names -main-file-name fbsd-kvm.c -static-define -mrelocation-model pic -pic-level 2 -target-feature +soft-float,+xgot -target-abi n64 -O2 -fcxx-exceptions -fexceptions -x c++ %s -o -
typedef int a;
#if 1
struct b {
  b();
}
#if c == d
#define e(f, g)                                                                \
  }                                                                            \
  catch (gdb_exception_##g)
#endif
;
struct gdb_exception_RETURN_MASK_ERROR {};
#endif
static a h() try {
  b i;
  e(, RETURN_MASK_ERROR) {}
  void j() {
    try {
      b i;
      e(, RETURN_MASK_ERROR) {}
      h();
    }
