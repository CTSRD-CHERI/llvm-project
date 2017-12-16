// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_is_p2aligned -DRETURNS_BOOL=1 -DPOW2=1 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_p2align_up   -DRETURNS_BOOL=0 -DPOW2=1 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_p2align_down -DRETURNS_BOOL=0 -DPOW2=1 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_is_aligned   -DRETURNS_BOOL=1 -DPOW2=0 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_align_up     -DRETURNS_BOOL=0 -DPOW2=0 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_align_down   -DRETURNS_BOOL=0 -DPOW2=0 %s -fsyntax-only -verify -pedantic

struct Aggregate {
  int i;
  int j;
};
enum Enum { EnumValue1,
            EnumValue2 };
typedef __SIZE_TYPE__ size_t;

void test_parameter_types(char *ptr, size_t size) {
  struct Aggregate agg;
  enum Enum e = EnumValue2;
  _Bool b = 0;

  // first parameter can be any pointer or integer type:
  (void)ALIGN_BUILTIN(ptr, 4);
  (void)ALIGN_BUILTIN(size, 2);
  (void)ALIGN_BUILTIN(12345, 2);
  (void)ALIGN_BUILTIN(agg, 2);    // expected-error {{operand of type 'struct Aggregate' where arithmetic or pointer type is required}}
  (void)ALIGN_BUILTIN(e, 2);      // expected-error {{operand of type 'enum Enum' where arithmetic or pointer type is required}}
  (void)ALIGN_BUILTIN(b, 2);      // expected-error {{operand of type '_Bool' where arithmetic or pointer type is required}}
  (void)ALIGN_BUILTIN((int)e, 2); // but with a cast it is fine
  (void)ALIGN_BUILTIN((int)b, 2); // but with a cast it is fine

  // second parameter doesn't have to be a constant for power of two versions:
#if POW2
  // second parameter must be an integer type (but not enum or _Bool)
  (void)ALIGN_BUILTIN(ptr, size);
  (void)ALIGN_BUILTIN(ptr, ptr);    // expected-error {{used type 'char *' where integer is required}}
  (void)ALIGN_BUILTIN(ptr, agg);    // expected-error {{used type 'struct Aggregate' where integer is required}}
  (void)ALIGN_BUILTIN(ptr, b);      // expected-error {{used type '_Bool' where integer is required}}
  (void)ALIGN_BUILTIN(ptr, e);      // expected-error {{used type 'enum Enum' where integer is required}}
  (void)ALIGN_BUILTIN(ptr, (int)e); // but with a cast it is fine
  (void)ALIGN_BUILTIN(ptr, (int)b); // but with a cast it is fine

  (void)ALIGN_BUILTIN(ptr, size);
  (void)ALIGN_BUILTIN(size, size);
#else
  (void)ALIGN_BUILTIN(ptr, (_Bool)1);     // expected-error {{used type '_Bool' where integer is required}}
  (void)ALIGN_BUILTIN(ptr, (enum Enum)4); // expected-error {{used type 'enum Enum' where integer is required}}
#endif
}

void test_return_type(void *ptr, int i, long l, __uintcap_t uintcap, void *__capability cap) {
  __extension__ typedef typeof(ALIGN_BUILTIN(ptr, 4)) result_type_ptr;
  __extension__ typedef typeof(ALIGN_BUILTIN(i, 4)) result_type_int;
  __extension__ typedef typeof(ALIGN_BUILTIN(l, 4)) result_type_long;
  __extension__ typedef typeof(ALIGN_BUILTIN(uintcap, 4)) result_type_uintcap;
  __extension__ typedef typeof(ALIGN_BUILTIN(cap, 4)) result_type_cap;
#if RETURNS_BOOL
  _Static_assert(__builtin_types_compatible_p(_Bool, result_type_ptr), "Should return bool");
  _Static_assert(__builtin_types_compatible_p(_Bool, result_type_int), "Should return bool");
  _Static_assert(__builtin_types_compatible_p(_Bool, result_type_long), "Should return bool");
  _Static_assert(__builtin_types_compatible_p(_Bool, result_type_uintcap), "Should return bool");
  _Static_assert(__builtin_types_compatible_p(_Bool, result_type_cap), "Should return bool");
#else
  _Static_assert(__builtin_types_compatible_p(void *, result_type_ptr), "Should return ");
  _Static_assert(__builtin_types_compatible_p(int, result_type_int), "Should return int");
  _Static_assert(__builtin_types_compatible_p(long, result_type_long), "Should return long");
  _Static_assert(__builtin_types_compatible_p(__uintcap_t, result_type_uintcap), "Should return uintcap_t");
  _Static_assert(__builtin_types_compatible_p(void *__capability, result_type_cap), "Should return capability");
#endif
}

void test_cheri_parameter_types(void *__capability cap, __uintcap_t uintcap, __intcap_t intcap) {
  // capabilities, uintcap_t and incap_t can also be aligned:
  (void)ALIGN_BUILTIN(cap, 4);     // not strictly an integer type but should be allowed too
  (void)ALIGN_BUILTIN(uintcap, 4); // not strictly an integer type but can also be aligned up/down
  (void)ALIGN_BUILTIN(intcap, 4);  // not strictly an integer type but can also be aligned up/down

  (void)ALIGN_BUILTIN(cap, cap); // expected-error{{used type 'void * __capability' where integer is required}}}
  // For (u)intptr_t compatibility we also allow __uintcap_t and __intcap_t as the aligment parameter type
  (void)ALIGN_BUILTIN(cap, (__uintcap_t)2); // not strictly an integer type but should be allowed as alignment value
  (void)ALIGN_BUILTIN(cap, (__intcap_t)2);  // not strictly an integer type but should be allowed as alignment value
#if POW2
  (void)ALIGN_BUILTIN(cap, uintcap); // not strictly an integer type but should be allowed as alignment value
  (void)ALIGN_BUILTIN(cap, intcap);  // not strictly an integer type but should be allowed as alignment value
#endif
}

#if POW2
void test_p2_range(char *ptr, void *__capability cap, size_t align) {
  (void)ALIGN_BUILTIN(ptr, 65); // expected-error {{requested power-of-two alignment 65 is not a value between 0 and 63}}
  int x = 1;
  (void)ALIGN_BUILTIN(x, 32); // expected-error {{requested power-of-two alignment 32 is not a value between 0 and 31}}
  (void)ALIGN_BUILTIN(x, 0);
#if RETURNS_BOOL
  // expected-warning@-2 {{checking whether a value is aligned to 1 byte is always true}}
#else
  // expected-warning@-4 {{aligning a value to 1 byte is always a noop}}
#endif
  (void)ALIGN_BUILTIN(x, -2); // expected-error {{requested power-of-two alignment -2 is not a value between 0 and 31}}

  char c = ' ';
  (void)ALIGN_BUILTIN(c, -1); // expected-error {{requested power-of-two alignment -1 is not a value between 0 and 7}}
  (void)ALIGN_BUILTIN(c, 8);  // expected-error {{requested power-of-two alignment 8 is not a value between 0 and 7}}

  // CHERI specific checks:
  // The range should still be 1 to 63 and not 127/255
  (void)ALIGN_BUILTIN(cap, -1); // expected-error {{requested power-of-two alignment -1 is not a value between 0 and 63}}
  (void)ALIGN_BUILTIN(cap, 65); // expected-error {{requested power-of-two alignment 65 is not a value between 0 and 63}}
  __uintcap_t uintcap = 3;
  __intcap_t intcap = 4;
  (void)ALIGN_BUILTIN(cap, uintcap); // not strictly an integer type but should be fine too
  (void)ALIGN_BUILTIN(cap, intcap);  // not strictly an integer type but should be fine too

  // check that we get the range right for __uintcap_t and __intcap_t
  (void)ALIGN_BUILTIN(uintcap, 64); // expected-error {{requested power-of-two alignment 64 is not a value between 0 and 63}}
  (void)ALIGN_BUILTIN(intcap, -5);  // expected-error {{requested power-of-two alignment -5 is not a value between 0 and 63}}
}
#else
void test_non_p2_values(char *ptr, void *__capability cap, size_t align) {
  int x = 1;
  (void)ALIGN_BUILTIN(ptr, 2);
  (void)ALIGN_BUILTIN(cap, 1024);
  (void)ALIGN_BUILTIN(x, 32);

  (void)ALIGN_BUILTIN(ptr, 0); // expected-error {{requested alignment must be 1 or greater}}
  (void)ALIGN_BUILTIN(ptr, 1);
#if RETURNS_BOOL
  // expected-warning@-2 {{checking whether a value is aligned to 1 byte is always true}}
#else
  // expected-warning@-4 {{aligning a value to 1 byte is always a noop}}
#endif
  (void)ALIGN_BUILTIN(ptr, 3); // expected-error {{requested alignment is not a power of 2}}
  (void)ALIGN_BUILTIN(x, 7);   // expected-error {{requested alignment is not a power of 2}}

  // check the maximum range for smaller types:
  __UINT8_TYPE__ c = ' ';

  (void)ALIGN_BUILTIN(c, 128);        // this is fine
  (void)ALIGN_BUILTIN(c, 256);        // expected-error {{requested alignment must be 128 or smaller}}
  (void)ALIGN_BUILTIN(x, 1ULL << 31); // this is also fine
  (void)ALIGN_BUILTIN(x, 1LL << 31);  // this is also fine
  __INT32_TYPE__ i32 = 3;
  __UINT32_TYPE__ u32 = 3;
  // Maximum is the same for int32 and uint32
  // XXXAR: should we differentiate?
  (void)ALIGN_BUILTIN(i32, 1ULL << 32); // expected-error {{requested alignment must be 2147483648 or smaller}}
  (void)ALIGN_BUILTIN(u32, 1ULL << 32); // expected-error {{requested alignment must be 2147483648 or smaller}}

  // XXXAR: for now we only allow integer constant expressions
  (void)ALIGN_BUILTIN(ptr, align);   // expected-error {{expression is not an integer constant}}
  (void)ALIGN_BUILTIN(align, align); // expected-error {{expression is not an integer constant}}

  // CHERI specific checks:
  // The range should still be 1 to 63 and not 127/255
  // TODO: can I generate a power of two bigger than 1 << 63? __int128_type?
  __uintcap_t uintcap = 3;
  __intcap_t intcap = 4;
  (void)ALIGN_BUILTIN(cap, align);     // expected-error {{expression is not an integer constant}}
  (void)ALIGN_BUILTIN(uintcap, align); // expected-error {{expression is not an integer constant}}
  (void)ALIGN_BUILTIN(intcap, align);  // expected-error {{expression is not an integer constant}}
}
#endif

// check that it can be used in constant expressions
void constant_expression(void) {
  _Static_assert(__builtin_is_aligned(1024, 512), "");
  _Static_assert(!__builtin_is_aligned(256, 512ULL), "");
  _Static_assert(__builtin_align_up(33, 32) == 64, "");
  _Static_assert(__builtin_align_down(33, 32) == 32, "");

  _Static_assert(__builtin_is_p2aligned(1024, 9), "");
  _Static_assert(!__builtin_is_p2aligned(3, 2), "");
  _Static_assert(__builtin_is_p2aligned(8, 3), "");
  _Static_assert(__builtin_p2align_up(33, 5) == 64, "");
  _Static_assert(__builtin_p2align_down(33, 5) == 32, "");
}
