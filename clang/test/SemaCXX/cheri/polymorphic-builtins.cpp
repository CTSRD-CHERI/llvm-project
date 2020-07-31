/// Check that (most) CHERI builtins are overloaded to allow any capability
/// pointer type or (u)intcap_t
// RUN: %cheri_purecap_cc1 %s -verify=expected,purecap
// RUN: %cheri_cc1 %s -verify=expected,hybrid,hybrid-cxx
// RUN: %cheri_cc1 -xc %s -verify=expected,hybrid,hybrid-c

typedef __SIZE_TYPE__ size_t;
struct Incomplete;

#ifdef __cplusplus
#define NULL nullptr

template <typename Cap, typename T = Cap>
void test_good(Cap x) {
  static_assert(__is_same(decltype(__builtin_cheri_address_get(x)), size_t), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}
  // hybrid-error@-2{{operand of type 'void (*)()' where capability is required}}
  // hybrid-error@-3{{operand of type 'void (*)()' where capability is required}}
  static_assert(__is_same(decltype(__builtin_cheri_address_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_base_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_bounds_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_bounds_set_exact(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_flags_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_flags_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_length_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_offset_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_offset_increment(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_offset_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_perms_and(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_perms_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_seal(x, nullptr)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_seal_entry(x)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_sealed_get(x)), bool), "");
  static_assert(__is_same(decltype(__builtin_cheri_subset_test(x, x)), bool), "");
  static_assert(__is_same(decltype(__builtin_cheri_tag_clear(x)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_tag_get(x)), bool), "");
  static_assert(__is_same(decltype(__builtin_cheri_type_get(x)), long), "");
  static_assert(__is_same(decltype(__builtin_cheri_unseal(x, nullptr)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_conditional_seal(x, nullptr)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_cap_type_copy(x, nullptr)), T), "");
}

void do_test() {
  test_good<void * __capability>(nullptr);
  test_good<const char * __capability>(nullptr);
  test_good<volatile int * __capability>(nullptr);
  test_good<Incomplete * __capability>(nullptr);
  test_good<__intcap_t>(1);
  test_good<__uintcap_t>(1);
  // This should decay to const char*:
  test_good<const char(&)[4], const char *>("foo"); // hybrid-note{{in instantiation of function template specialization 'test_good<char const (&)[4], const char *>' requested here}}
  // passing function pointers should also be allowed
  test_good(&do_test);                            // hybrid-note{{in instantiation of function template specialization 'test_good<void (*)(), void (*)()>' requested here}}
  test_good<void(void), void (*)(void)>(do_test); // hybrid-note{{in instantiation of function template specialization 'test_good<void (), void (*)()>' requested here}}
}
#else
#define NULL ((void *)0)

#define __is_same __builtin_types_compatible_p
#define static_assert _Static_assert
#endif // __cplusplus

// Check handling of NULL pointer arguments:
void test_nullptr(__uintcap_t caparg) {
#ifdef __cplusplus
  // nullptr should be fine:
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(nullptr, 1)), void *__capability), "");
  // Same for the GNU __null extension:
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(__null, 1)), void *__capability), "");
#endif
  // Canonical NULL macros should be fine in both C and C++:
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(NULL, 1)), void *__capability), "");

  // The canonical C definition of NULL, `((void*)0)`, should be implicitly converted
  // from void * -> void * __capability in C, but not in C++ since '(void*)0' is not
  // treated as a null pointer constant in C++.
  static_assert(__is_same(__typeof__(__builtin_cheri_perms_and((void *)0, 1)), void *__capability), "");
  // hybrid-cxx-error@-1{{operand of type 'void *' where capability is required}}

  // The expression (const char*)0 is never a null pointer constant (not even in C).
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set((const char *)0, 1)), const char *__capability), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}

  // Literal zero is a null pointer constant in both C and C++ and therefore maps to void* __capability
  // This could result in accidentally swapping arguments for the __uintcap_t overloads
  // of e.g. __builtin_cheri_flags_set(), but those problems should be caught by the return
  // type being void* __capability instead of __uintcap_t.
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(caparg, 0)), __uintcap_t), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((void *__capability)0, caparg)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((const int *__capability)0, caparg)), const int *__capability), "");

  // A literal zero should also be treated as a capability
#define OLD_CXX_NULL 0
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(OLD_CXX_NULL, caparg)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((void *__capability)OLD_CXX_NULL, caparg)), void *__capability), "");
  // However, casting zero to a non-void* type should be an error in hybrid mode since that is not a
  // valid null pointer constant:
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((int *)0, caparg)), int *__capability), "");
  // hybrid-error@-1{{operand of type 'int *' where capability is required}}
  // The correct way to do this is to cast to 'int * __capabiliyt' instead of 'int *'
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((int *__capability)0, caparg)), int *__capability), "");
}

void test_buildcap(struct Incomplete *__capability authcap, __uintcap_t ubits, __intcap_t sbits, char *__capability charpbits) {
  // void * __capability __builtin_cheri_cap_build(<anycap> auth, __(u)intcap_t bits)
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, ubits)), void *__capability), ""); // okay
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, sbits)), void *__capability), ""); // okay
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, charpbits)), void *__capability), "");
  // purecap-error@-1{{cannot initialize a parameter of type '__uintcap_t' with an rvalue of type 'char *'}}
  // hybrid-cxx-error@-2{{cannot initialize a parameter of type '__uintcap_t' with an rvalue of type 'char * __capability'}}
  // hybrid-c-warning@-3{{incompatible pointer to integer conversion passing 'char * __capability' to parameter of type '__uintcap_t'}}
  // Note: we may want to allow any capability type as the second argument if this restriction becomes annoying
}

void test_decay() {
  const char *x = __builtin_cheri_bounds_set("abc", 1);
  // hybrid-cxx-error@-1{{operand of type 'const char *' where capability is required}}
  // hybrid-c-error@-2{{operand of type 'char *' where capability is required}}
  void (*f1)(void) = __builtin_cheri_bounds_set(test_decay, 1);
  // hybrid-error@-1{{operand of type 'void (*)()' where capability is required}}
  void (*f2)(void) = __builtin_cheri_bounds_set(&test_decay, 1);
  // hybrid-error@-1{{operand of type 'void (*)()' where capability is required}}
}

void test_bad() {
  long x = 0;
  (void)__builtin_cheri_address_get(x);         // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_address_set(x, 1);      // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_base_get(x);            // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_bounds_set(x, 1);       // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_bounds_set_exact(x, 1); // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_flags_set(x, 1);        // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_flags_get(x);           // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_length_get(x);          // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_offset_get(x);          // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_offset_increment(x, 1); // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_offset_set(x, 1);       // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_perms_and(x, 1);        // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_perms_get(x);           // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_seal(x, NULL);          // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_seal_entry(x);          // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_sealed_get(x);          // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_subset_test(x, x);      // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_tag_clear(x);           // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_tag_get(x);             // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_type_get(x);            // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_unseal(x, NULL);        // expected-error{{operand of type 'long' where capability is required}}

  // Check that we don't promote integer constants
  (void)__builtin_cheri_address_get(123);         // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_address_set(123, 1);      // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_base_get(123);            // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_bounds_set(123, 1);       // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_bounds_set_exact(123, 1); // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_flags_set(123, 1);        // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_flags_get(123);           // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_length_get(123);          // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_offset_get(123);          // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_offset_increment(123, 1); // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_offset_set(123, 1);       // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_perms_and(123, 1);        // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_perms_get(123);           // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_seal(123, NULL);          // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_seal_entry(123);          // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_sealed_get(123);          // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_subset_test(123, 123);    // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_tag_clear(123);           // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_tag_get(123);             // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_type_get(123);            // expected-error{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_unseal(123, NULL);        // expected-error{{operand of type 'int' where capability is required}}
  // or floating-point ones
  (void)__builtin_cheri_address_get(1.0f);         // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_address_set(1.0f, 1);      // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_base_get(1.0f);            // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_bounds_set(1.0f, 1);       // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_bounds_set_exact(1.0f, 1); // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_flags_set(1.0f, 1);        // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_flags_get(1.0f);           // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_length_get(1.0f);          // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_offset_get(1.0f);          // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_offset_increment(1.0f, 1); // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_offset_set(1.0f, 1);       // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_perms_and(1.0f, 1);        // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_perms_get(1.0f);           // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_seal(1.0f, NULL);          // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_seal_entry(1.0f);          // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_sealed_get(1.0f);          // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_subset_test(1.0f, 1.0f);   // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_tag_clear(1.0f);           // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_tag_get(1.0f);             // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_type_get(1.0f);            // expected-error{{operand of type 'float' where capability is required}}
  (void)__builtin_cheri_unseal(1.0f, NULL);        // expected-error{{operand of type 'float' where capability is required}}
}
