// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify=expected,purecap
// RUN: %cheri_cc1 -fsyntax-only %s -verify=expected,hybrid
// We previously asserted in the capability conversion detection code when
// encountering a GCC union cast extension (CK_ToUnion).
// https://github.com/CTSRD-CHERI/llvm-project/issues/446

typedef union {
  void *__capability ptr;
  int i;
} CapIntUnion;

typedef union {
  void *ptr;
  int i;
} NonCapIntUnion;

typedef union {
  int i;
  float f;
} InvalidUnion;

CapIntUnion matching_type1(void *__capability b) {
  return (CapIntUnion)b;
}
NonCapIntUnion matching_type2(void *b) {
  return (NonCapIntUnion)b;
}

NonCapIntUnion cap_conversion1(void *__capability b) {
  return (NonCapIntUnion)b; // hybrid-error{{cast to union type from type 'void * __capability' not present in union}}
}
CapIntUnion cap_conversion2(void *b) {
  return (CapIntUnion)b; // hybrid-error{{cast to union type from type 'void *' not present in union}}
}

InvalidUnion bad_conversion1(void *__capability b) {
  return (InvalidUnion)b;
  // purecap-error@-1{{cast to union type from type 'void *' not present in union}}
  // hybrid-error@-2{{cast to union type from type 'void * __capability' not present in union}}
}
InvalidUnion bad_conversion2(void *b) {
  return (InvalidUnion)b; // expected-error{{cast to union type from type 'void *' not present in union}}
}
