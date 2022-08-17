// RUN: %riscv64_cheri_cc1 %s -fsyntax-only -verify=expected,expected-c,hybrid,hybrid-c
// RUN: %riscv64_cheri_purecap_cc1 %s -fsyntax-only -verify=expected,expected-c,purecap,purecap-c
// RUN: %riscv64_cheri_cc1 -x c++ %s -fsyntax-only -verify=expected,expected-cxx,hybrid,hybrid-cxx
// RUN: %riscv64_cheri_purecap_cc1 -x c++ %s -fsyntax-only -verify=expected,expected-cxx,purecap,purecap-cxx

// TODO: Capabilities are being overly-restricted here compared with integers
// (both integral pointers and real integers) and give strange errors, such as
// C++ ones in C code.

extern char foo[];

struct {
  int x;
} ints[] = {
  {(void *)0},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void *'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // expected-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  {(void *)42},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void *'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __attribute__((cheri_no_provenance))'}}
  {(void *)((long)__INT_MAX__ + 1)},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void *'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __attribute__((cheri_no_provenance))'}}
  {(void *)foo},
  // expected-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void *'}}
  // expected-c-error@-2 {{initializer element is not a compile-time constant}}
  // expected-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  {(void * __capability)0},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __capability __attribute__((cheri_no_provenance))'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __capability'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  {(void * __capability)42},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __capability __attribute__((cheri_no_provenance))'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __capability __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __attribute__((cheri_no_provenance))'}}
  {(void * __capability)((long)__INT_MAX__ + 1)},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __capability __attribute__((cheri_no_provenance))'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __capability __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __attribute__((cheri_no_provenance))'}}
  {(void * __capability)foo},
  // hybrid-c-warning@-1 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void * __capability'}}
  // purecap-c-warning@-2 {{incompatible pointer to integer conversion initializing 'int' with an expression of type 'void *'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void * __capability'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'int' with an rvalue of type 'void *'}}
  {(__intcap)0},
  {(__intcap)42},
  {(__intcap)((long)__INT_MAX__ + 1)},
  // expected-c-warning@-1 {{implicit conversion from '__intcap __attribute__((cheri_no_provenance))' to 'int' changes value from 2147483648 to -2147483648}}
  // expected-cxx-error@-2 {{constant expression evaluates to 2147483648 which cannot be narrowed to type 'int'}}
  // expected-cxx-note@-3 {{insert an explicit cast to silence this issue}}
#ifdef __cplusplus
  {__null},
  {(__intcap)__null},
#endif
};

struct {
  void *p;
} ptrs[] = {
  {(void *)0},
  {(void *)42},
  {(void *)((long)__INT_MAX__ + 1)},
  {(void *)foo},
  {(void * __capability)0},
  // hybrid-cxx-error@-1 {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  {(void * __capability)42},
  // hybrid-c-error@-1 {{converting capability type 'void * __capability __attribute__((cheri_no_provenance))' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  // hybrid-cxx-error@-2 {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  {(void * __capability)((long)__INT_MAX__ + 1)},
  // hybrid-c-error@-1 {{converting capability type 'void * __capability __attribute__((cheri_no_provenance))' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  // hybrid-cxx-error@-2 {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  {(void * __capability)foo},
  // hybrid-error@-1 {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  {(__intcap)0},
  // expected-c-warning@-1 {{expression which evaluates to zero treated as a null pointer constant of type 'void *'}}
  // expected-cxx-error@-2 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  {(__intcap)42},
  // expected-c-warning@-1 {{incompatible integer to pointer conversion initializing 'void *' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // expected-cxx-error@-2 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  {(__intcap)((long)__INT_MAX__ + 1)},
  // expected-c-warning@-1 {{incompatible integer to pointer conversion initializing 'void *' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // expected-cxx-error@-2 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
#ifdef __cplusplus
  {__null},
  {(__intcap)__null},
  // expected-cxx-error@-1 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
#endif
};

struct {
  void * __capability p;
} caps[] = {
  {(void *)0},
  // hybrid-cxx-warning@-1 {{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  {(void *)42},
  // hybrid-warning@-1 {{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  {(void *)((long)__INT_MAX__ + 1)},
  // hybrid-warning@-1 {{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  {(void *)foo},
  // hybrid-warning@-1 {{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
  {(void * __capability)0},
  {(void * __capability)42},
  {(void * __capability)((long)__INT_MAX__ + 1)},
  {(void * __capability)foo},
  {(__intcap)0},
  // hybrid-c-warning@-1 {{expression which evaluates to zero treated as a null pointer constant of type 'void * __capability'}}
  // purecap-c-warning@-2 {{expression which evaluates to zero treated as a null pointer constant of type 'void *'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'void * __capability' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  {(__intcap)42},
  // hybrid-c-warning@-1 {{incompatible integer to pointer conversion initializing 'void * __capability' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-c-warning@-2 {{incompatible integer to pointer conversion initializing 'void *' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'void * __capability' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  {(__intcap)((long)__INT_MAX__ + 1)},
  // hybrid-c-warning@-1 {{incompatible integer to pointer conversion initializing 'void * __capability' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-c-warning@-2 {{incompatible integer to pointer conversion initializing 'void *' with an expression of type '__intcap __attribute__((cheri_no_provenance))'}}
  // hybrid-cxx-error@-3 {{cannot initialize a member subobject of type 'void * __capability' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-4 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
#ifdef __cplusplus
  {__null},
  {(__intcap)__null},
  // hybrid-cxx-error@-1 {{cannot initialize a member subobject of type 'void * __capability' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
  // purecap-cxx-error@-2 {{cannot initialize a member subobject of type 'void *' with an rvalue of type '__intcap __attribute__((cheri_no_provenance))'}}
#endif
};
