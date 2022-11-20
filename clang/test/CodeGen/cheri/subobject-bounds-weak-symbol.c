// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O0 -std=c11 -emit-llvm -xc %s -o /dev/null -Rcheri-subobject-bounds -verify

// Lots of code checks for &_DYNAMIC == NULL to see if it is statically linked (and
// declares it as an int or char since it cares only about null vs non-null.
// See https://github.com/CTSRD-CHERI/llvm-project/issues/317

extern int _DYNAMIC1 __attribute__((weak));

_Bool is_static_attribute_weak(void) {
  return &_DYNAMIC1 == 0;
  // expected-remark@-1{{not setting bounds for pointer to 'int' (referenced value is a weak symbol and could therefore be NULL)}}
}

extern int _DYNAMIC2;
#pragma weak _DYNAMIC2

_Bool is_static_pragma_weak(void) {
  return &_DYNAMIC2 == 0;
  // expected-remark@-1{{not setting bounds for pointer to 'int' (referenced value is a weak symbol and could therefore be NULL)}}
}

extern int _DYNAMIC3;

_Bool is_static_not_weak(void) {
  return &_DYNAMIC3 == 0;
  // expected-warning@-1{{comparison of address of '_DYNAMIC3' equal to a null pointer is always false}}
  // expected-remark@-2{{setting bounds for pointer to 'int' to 4 bytes}}
}

extern int weak_array[5];
#pragma weak weak_array

int test_weak_array(void) {
  if (weak_array) {
    // expected-remark@-1{{not setting bounds for array decay on 'int[5]' (referenced value is a weak symbol and could therefore be NULL)}}
    return weak_array[0];
    // expected-remark@-1{{not setting bounds for array subscript on 'int[5]' (referenced value is a weak symbol and could therefore be NULL)}}
  }
  return 0;
}
