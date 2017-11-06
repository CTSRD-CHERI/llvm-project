// RUN: %cheri_cc1 -target-abi n64 -O2 -std=gnu99 -fconst-strings -vectorize-loops -vectorize-slp  -o - -O0 -emit-llvm  %s
// XFAIL: *
// This crashes crashes during code gen
// XXXAR: do we want to make this code an error by default?

struct a {
  void *__capability b;
} * c;
_Bool cheritest_libcheri_userfn_getstack(struct a *csfp) {
  return csfp->b != c;
}
