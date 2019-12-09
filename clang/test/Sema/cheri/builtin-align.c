// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_is_aligned -DRETURNS_BOOL=1 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_align_up   -DRETURNS_BOOL=0 %s -fsyntax-only -verify -pedantic
// RUN: %cheri_cc1 -DALIGN_BUILTIN=__builtin_align_down -DRETURNS_BOOL=0 %s -fsyntax-only -verify -pedantic

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

  (void)ALIGN_BUILTIN(cap, uintcap); // not strictly an integer type but should be allowed as alignment value
  (void)ALIGN_BUILTIN(cap, intcap);  // not strictly an integer type but should be allowed as alignment value
}

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
  // expected-warning@-4 {{aligning a value to 1 byte is a no-op}}
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

  const int bad_align = 8 + 1;
  // bad_align += 1;
  (void)ALIGN_BUILTIN(ptr, bad_align); // expected-error {{requested alignment is not a power of 2}}

  // CHERI specific checks:
  // The range should still be 1 to 63 and not 127/255
  __intcap_t intcap = 1;
  __uintcap_t uintcap = 2;
  (void)ALIGN_BUILTIN(cap, ((__int128)1) << 65); // expected-error {{requested alignment must be 9223372036854775808 or smaller}}
  (void)ALIGN_BUILTIN(uintcap, ((__int128)1) << 65); // expected-error {{requested alignment must be 9223372036854775808 or smaller}}
  (void)ALIGN_BUILTIN(intcap, ((__int128)1) << 65);  // expected-error {{requested alignment must be 9223372036854775808 or smaller}}

}

// check that it can be used in constant expressions
void constant_expression(int x) {
  _Static_assert(__builtin_is_aligned(1024, 512), "");
  _Static_assert(!__builtin_is_aligned(256, 512ULL), "");
  _Static_assert(__builtin_align_up(33, 32) == 64, "");
  _Static_assert(__builtin_align_down(33, 32) == 32, "");

  // but not if one of the arguments isn't constant
  _Static_assert(ALIGN_BUILTIN(33, x) != 100, ""); // expected-error {{static_assert expression is not an integral constant expression}}
  _Static_assert(ALIGN_BUILTIN(x, 4) != 100, ""); // expected-error {{static_assert expression is not an integral constant expression}}
}

// Check that it is a constant expression that can be assigned to globals:
int global1 = __builtin_align_down(33, 8);
int global2 = __builtin_align_up(33, 8);
_Bool global3 = __builtin_is_aligned(33, 8);

// Check that we can use it on array types:
// https://github.com/CTSRD-CHERI/llvm-project/issues/328
extern void test_ptr(char *c);
char *test_array(void) {
  char buf[1024];
  // Array to pointer decay should work:
  test_ptr(__builtin_align_up(buf, 16));
  // Check that the return type decayed (if not this will result in a warning)
  _Bool decayed = ((__typeof__(__builtin_align_up(buf, 16)))0) == ((char*)0);
  // but not function to function-pointer:
  test_ptr(__builtin_align_up(test_array, 16)); // expected-error{{operand of type 'char *(void)' where arithmetic or pointer type is required}}
}
