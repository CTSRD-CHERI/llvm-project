/// Check implicit conversions between capability an non-capability are no permitted in returns
// RUN: %clang_cc1 -Wcheri-pointer-conversion -verify=cxx,expected,nocheri -fsyntax-only %s
// RUN: %cheri_cc1 -Wcheri-pointer-conversion -verify=cxx,expected,hybrid -fsyntax-only %s
// RUN: %cheri_purecap_cc1 -Wcheri-pointer-conversion -verify=cxx,expected,purecap -fsyntax-only  %s
/// Also check when compiling C:
// RUN: %clang_cc1 -Wall -Wcheri-pointer-conversion -verify=c,expected,nocheri,nocheri-c -x c -fsyntax-only %s
// RUN: %cheri_cc1 -Wall -Wcheri-pointer-conversion -verify=c,expected,hybrid,hybrid-c -x c -fsyntax-only %s
// RUN: %cheri_purecap_cc1 -Wall -Wcheri-pointer-conversion -verify=c,expected,purecap,purecap-c -x c -fsyntax-only %s

#if __has_feature(capabilities)
typedef unsigned __intcap uintcap_t;
#else
typedef unsigned long uintcap_t;
#define __capability
#endif

void *cast_long_to_intptr_implicit(unsigned long l) {
  return l;
  // cxx-error@-1{{cannot initialize return object of type 'void *' with an lvalue of type 'unsigned long'}}
  // C allows this behaviour, but warns by default
  // c-warning@-3{{incompatible integer to pointer conversion returning 'unsigned long' from a function with result type 'void *'}}
}

void *__capability cast_long_to_capptr_implicit(unsigned long l) {
  return l;
  // cxx-error-re@-1{{cannot initialize return object of type 'void *{{( __capability)?}}' with an lvalue of type 'unsigned long'}}
  // C allows this behaviour, but warns by default
  // c-warning-re@-3{{incompatible integer to pointer conversion returning 'unsigned long' from a function with result type 'void *{{( __capability)?}}'}}
  // FIXME: should warn in purecap mode about lack of provenance
}

void *cast_uintcap_to_intptr_implicit(uintcap_t cap) {
  return cap;
  // cxx-error@-1{{cannot initialize return object of type 'void *' with an lvalue of type 'uintcap_t'}}
  // C allows this behaviour, but warns by default
  // c-warning-re@-3{{incompatible integer to pointer conversion returning 'uintcap_t' (aka 'unsigned {{(long|__intcap)}}') from a function with result type 'void *'}}
}

void *__capability cast_uintcap_to_capptr_implicit(uintcap_t cap) {
  return cap;
  // cxx-error-re@-1{{cannot initialize return object of type 'void *{{( __capability)?}}' with an lvalue of type 'uintcap_t'}}
  // C allows this behaviour, but warns by default
  // c-warning-re@-3{{incompatible integer to pointer conversion returning 'uintcap_t' (aka 'unsigned {{(long|__intcap)}}') from a function with result type 'void *{{( __capability)?}}'}}
}

void *cast_capptr_to_intptr_implicit(void *__capability cap) {
  return cap; // hybrid-error{{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
}

void *__capability cast_intptr_to_capptr_implicit(void *ptr) {
  return ptr; // hybrid-warning{{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast;}}
}
