/// Check that (most) CHERI builtins are overloaded to allow any capability
/// pointer type or (u)intcap_t
// RUN: %cheri_purecap_cc1 %s -verify=expected,purecap,purecap-cxx
// RUN: %cheri_purecap_cc1 -xc %s -verify=expected,purecap,purecap-c
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
  static_assert(__is_same(decltype(__builtin_cheri_base_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_bounds_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_bounds_set_exact(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_equal_exact(x, x)), bool), "");
  static_assert(__is_same(decltype(__builtin_cheri_flags_set(x, 1)), T), "");
  static_assert(__is_same(decltype(__builtin_cheri_flags_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_length_get(x)), size_t), "");
  static_assert(__is_same(decltype(__builtin_cheri_offset_get(x)), size_t), "");
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
  static_assert(__is_same(decltype(__builtin_cheri_perms_check(x, 1)), void), "");
  static_assert(__is_same(decltype(__builtin_cheri_type_check(x, x)), void), "");
}

void do_test() {
  test_good<void * __capability>(nullptr);
  test_good<const char * __capability>(nullptr);
  test_good<volatile int * __capability>(nullptr);
  test_good<Incomplete * __capability>(nullptr);
  test_good<__intcap>(1);
  test_good<unsigned __intcap>(1);
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
void test_nullptr(unsigned __intcap caparg) {
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
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set((const char *)0, 1)), const char *__capability), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}

  // Literal zero is a null pointer constant in both C and C++ and therefore maps to void* __capability
  // This could result in accidentally swapping arguments for the unsigned __intcap overloads
  // of e.g. __builtin_cheri_flags_set(), but those problems should be caught by the return
  // type being void* __capability instead of unsigned __intcap.
  static_assert(__is_same(__typeof__(__builtin_cheri_flags_set(caparg, 0)), unsigned __intcap), "");
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

void test_buildcap(struct Incomplete *__capability authcap, unsigned __intcap ubits, __intcap sbits, char *__capability charpbits) {
  // void * __capability __builtin_cheri_cap_build(<anycap> auth, __(u)intcap_t bits)
  // Note: we may want to allow any capability type as the second argument if this restriction becomes annoying
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, ubits)), void *__capability), ""); // okay
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, sbits)), void *__capability), ""); // okay
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_build(authcap, charpbits)), void *__capability), "");
  // purecap-error@-1{{used type 'char *' where integer is required}}
  // hybrid-error@-2{{used type 'char * __capability' where integer is required}}
}

void test_incoffset(long *__capability lcap, volatile int *__capability vicap, float *fptr) {
#ifdef __cplusplus
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment("abc", 1)), const void *__capability), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}
#else
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment("abc", 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'char *' where capability is required}}
#endif
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment(lcap, 1)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment(vicap, 1)), volatile void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment(fptr, 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'float *' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment((__intcap)2, 1)), __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment((unsigned __intcap)3, 1)), unsigned __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment(3, 1)), unsigned __intcap), "");
  // expected-error@-1{{operand of type 'int' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_increment(0, 1)), void *__capability), ""); // NULL literal
}

void test_setoffset(long *__capability lcap, volatile int *__capability vicap, float *fptr) {
#ifdef __cplusplus
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set("abc", 1)), const void *__capability), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}
#else
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set("abc", 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'char *' where capability is required}}
#endif
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set(lcap, 1)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set(vicap, 1)), volatile void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set(fptr, 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'float *' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set((__intcap)2, 1)), __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set((unsigned __intcap)3, 1)), unsigned __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set(3, 1)), unsigned __intcap), "");
  // expected-error@-1{{operand of type 'int' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_offset_set(0, 1)), void *__capability), ""); // NULL literal
}

void test_setaddr(long *__capability lcap, volatile int *__capability vicap, float *fptr) {
#ifdef __cplusplus
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set("abc", 1)), const void *__capability), "");
  // hybrid-error@-1{{operand of type 'const char *' where capability is required}}
#else
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set("abc", 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'char *' where capability is required}}
#endif
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set(lcap, 1)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set(vicap, 1)), volatile void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set(fptr, 1)), void *__capability), "");
  // hybrid-error@-1{{operand of type 'float *' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set((__intcap)2, 1)), __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set((unsigned __intcap)3, 1)), unsigned __intcap), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set(3, 1)), unsigned __intcap), "");
  // expected-error@-1{{operand of type 'int' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_address_set(0, 1)), void *__capability), ""); // NULL literal
}

void cap_to_pointer(struct Incomplete *__capability authcap, void *integerptr, void *__capability capptr, unsigned __intcap ui, __intcap si) {
#ifdef __CHERI_PURE_CAPABILITY__
  // not available in purecap
  (void)__builtin_cheri_cap_to_pointer(authcap, capptr); // purecap-error{{builtin is not supported on this target}}
#else
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, 0)), void *), ""); // 0 is a NULL pointer -> void*
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, NULL)), void *), "");
  // (u)intcap_t -> (u)intptr_t
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, ui)), unsigned long), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, si)), signed long), "");

  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, (const char *__capability)capptr)), const char *), ""); // okay
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_to_pointer(authcap, capptr)), void *), "");
  (void)__builtin_cheri_cap_to_pointer(authcap, integerptr); // expected-error{{operand of type 'void *' where capability is required}}
  (void)__builtin_cheri_cap_to_pointer(integerptr, NULL);    // expected-error{{operand of type 'void *' where capability is required}}
#endif
}

void cap_from_pointer(struct Incomplete *__capability authcap, const int *integerptr, const int *__capability capptr, unsigned __intcap ui, __intcap si, long l, int i) {
  // Using an integer type results in a void * __capability return value
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(authcap, i)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(authcap, l)), void *__capability), "");
  // However, we reject __(u)intcap_t since that is a capability type:
  (void)__builtin_cheri_cap_from_pointer(authcap, ui); // expected-error-re{{type 'unsigned __intcap' where {{(arithmetic or pointer type)|integer}} is required}}
  (void)__builtin_cheri_cap_from_pointer(authcap, si); // expected-error-re{{type '__intcap' where {{(arithmetic or pointer type)|integer}} is required}}

  // NULL is error in C++ (hybrid+purecap) since the operand is nullptr_t, and in
  // purecap C it's also an error since NULL is defined as (void*)0.
  (void)__builtin_cheri_cap_from_pointer(authcap, NULL);
  // purecap-cxx-error@-1{{used type 'std::nullptr_t' where integer is required}}
  // purecap-c-error@-2{{used type 'void * __attribute__((cheri_no_provenance))' where integer is required}}
  // hybrid-cxx-error@-3{{operand of type 'std::nullptr_t' where arithmetic or pointer type is required}}
  // (void*)0 is only an error in purecap mode:
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(authcap, (void *)0)), void *__capability), "");
  // purecap-cxx-error@-1{{used type 'void *' where integer is required}}
  // purecap-c-error@-2{{used type 'void * __attribute__((cheri_no_provenance))' where integer is required}}
  // Literal zero is fine since it's an integer (and therefore the return type is void* __capability):
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(authcap, 0)), void *__capability), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(authcap, integerptr)), const int *__capability), "");
  // purecap-error@-1{{used type 'const int *' where integer is required}}
  (void)__builtin_cheri_cap_from_pointer(authcap, capptr);
  // purecap-error@-1{{used type 'const int *' where integer is required}}
  // hybrid-error@-2{{operand of type 'const int * __capability' where arithmetic or pointer type is required}}

  // authorizing cap must be a capability:
  (void)__builtin_cheri_cap_from_pointer(integerptr, l);
  // hybrid-error@-1{{operand of type 'const int *' where capability is required}}
  (void)__builtin_cheri_cap_from_pointer(l, l);
  // expected-error@-1{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_cap_from_pointer(i, l);
  // expected-error@-1{{operand of type 'int' where capability is required}}
  (void)__builtin_cheri_cap_from_pointer(1, l);
  // expected-error@-1{{operand of type 'int' where capability is required}}
  // literal zero is a NULL pointer constant so it's accepted as the auth argument
  static_assert(__is_same(__typeof__(__builtin_cheri_cap_from_pointer(0, l)), void *__capability), "");
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

void check_perms_type_check_builtins(const int *__capability capptr, unsigned __intcap uintcap, long l) {
  static_assert(__is_same(__typeof__(__builtin_cheri_perms_check(capptr, 1)), void), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_perms_check(uintcap, 1)), void), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_perms_check(l, 1)), void), "");
  // expected-error@-1{{operand of type 'long' where capability is required}}

  static_assert(__is_same(__typeof__(__builtin_cheri_type_check(uintcap, capptr)), void), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_type_check(capptr, uintcap)), void), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_type_check(capptr, 0)), void), "");
  static_assert(__is_same(__typeof__(__builtin_cheri_type_check(capptr, l)), void), "");
  // expected-error@-1{{operand of type 'long' where capability is required}}
  static_assert(__is_same(__typeof__(__builtin_cheri_type_check(l, capptr)), void), "");
  // expected-error@-1{{operand of type 'long' where capability is required}}
}

void test_bad() {
  long x = 0;
  (void)__builtin_cheri_address_get(x);         // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_address_set(x, 1);      // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_base_get(x);            // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_bounds_set(x, 1);       // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_bounds_set_exact(x, 1); // expected-error{{operand of type 'long' where capability is required}}
  (void)__builtin_cheri_equal_exact(x, x);      // expected-error{{operand of type 'long' where capability is required}}
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
  (void)__builtin_cheri_equal_exact(123, 123);    // expected-error{{operand of type 'int' where capability is required}}
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
  (void)__builtin_cheri_equal_exact(1.0f, 1.0f);   // expected-error{{operand of type 'float' where capability is required}}
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
