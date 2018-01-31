// RUN: %cheri_cc1 -target-abi n64 -O2 -std=gnu99 -fconst-strings -vectorize-loops -vectorize-slp  -o - -O0 -emit-llvm -verify %s

struct a {
  void *__capability b;
} * c;
_Bool cheritest_libcheri_userfn_getstack(struct a *csfp) {
  return csfp->b != c; // expected-error {{converting non-capability type 'struct a *' to capability type 'void * __capability' without an explicit cast}} expected-error {{comparison between integer pointer and capability ('void * __capability' and 'struct a *')}}
}
