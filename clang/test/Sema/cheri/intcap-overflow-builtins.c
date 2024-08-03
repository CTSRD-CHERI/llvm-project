// RUN: %cheri_cc1 %s -Wcheri-provenance -Wcheri-provenance-pedantic -fsyntax-only -verify

_Bool add(__intcap a, __intcap b, __intcap *c) {
  return __builtin_add_overflow(a, b, c);
  // expected-warning@-1 {{binary expression on capability types '__intcap' and '__intcap'; it is not clear which should be used as the source of provenance; currently provenance is inherited from the left-hand side}}
}

_Bool add_lhs(__intcap a, int b, __intcap *c) {
  return __builtin_add_overflow(a, b, c);
}

_Bool add_rhs(int a, __intcap b, __intcap *c) {
  return __builtin_add_overflow(a, b, c);
}

_Bool add_promote(int a, int b, __intcap *c) {
  return __builtin_add_overflow(a, b, c);
}

_Bool add_demote(__intcap a, __intcap b, int *c) {
  return __builtin_add_overflow(a, b, c);
  // expected-warning@-1 {{binary expression on capability types '__intcap' and '__intcap'; it is not clear which should be used as the source of provenance; currently provenance is inherited from the left-hand side}}
}

_Bool add_provenance(int a, __intcap b, __intcap *c) {
  return __builtin_add_overflow((__intcap)a, b, c);
}

_Bool sub(__intcap a, __intcap b, __intcap *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool sub_lhs(__intcap a, int b, __intcap *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool sub_rhs(int a, __intcap b, __intcap *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool sub_promote(int a, int b, __intcap *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool sub_demote(__intcap a, __intcap b, int *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool sub_provenance(int a, __intcap b, __intcap *c) {
  return __builtin_sub_overflow((__intcap)a, b, c);
}

_Bool mul(__intcap a, __intcap b, __intcap *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
  // expected-warning@-1 {{binary expression on capability types '__intcap' and '__intcap'; it is not clear which should be used as the source of provenance; currently provenance is inherited from the left-hand side}}
}

_Bool mul_lhs(__intcap a, int b, __intcap *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
}

_Bool mul_rhs(int a, __intcap b, __intcap *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
}

_Bool mul_promote(int a, int b, __intcap *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
}

_Bool mul_demote(__intcap a, __intcap b, int *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
  // expected-warning@-1 {{binary expression on capability types '__intcap' and '__intcap'; it is not clear which should be used as the source of provenance; currently provenance is inherited from the left-hand side}}
}

_Bool mul_provenance(int a, __intcap b, __intcap *c) {
  return __builtin_mul_overflow((__intcap)a, b, c);
}
