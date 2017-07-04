// RUN: %clang_cc1 -triple cheri-unknown-freebsd -std=c11 -o - %s -fsyntax-only -verify

#define NULL ((void*)0)
// These warnings only happen in the hybrid ABI
struct foo {
  void* __capability cap;
  void* ptr;
};

void test_cap_to_ptr(void* __capability a) {
  void* non_cap = a; // expected-error {{initializing 'void *' with an expression of incompatible type 'void * __capability'}}
  // TODO: These should warn as well
  struct foo f = {a, a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  struct foo f2;
  f2 = (struct foo){a, NULL}; // this is fine
  f2 = (struct foo){a, a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  struct foo f3 = {NULL, NULL};

  // C99 designated initializer:
  struct foo f4 = { .cap = a, .ptr = NULL};
  struct foo f5 = { .cap = a, .ptr = a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
}

// check arrays:
void test_arrays(void* __capability cap) {
  int* ptr = NULL;
  void* ptr_array[3] = { NULL, cap, ptr }; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  // GCC range init:
  void* ptr_array2[3] = { [0 ... 1] = NULL, cap }; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  void* ptr_array3[3] = { [0 ... 2] = cap }; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  // TODO: this should probably warn about pointer -> cap conversion

  void* __capability cap_array[3] = { NULL, cap, ptr };  // expected-error {{initializing 'void * __capability' with an expression of incompatible type 'int *'}}

  struct foo foo_array[5] = {
      {cap, NULL}, // no-error
      {cap, cap}, // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
      {.cap = NULL, .ptr = NULL}, // no-error
      [4] = {.cap = cap, .ptr = cap} // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  };
}

union foo_union {
  void* ptr;
  long l;
};

void test_union(void* __capability a) {
  union foo_union u = {a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  u = (union foo_union){a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  union foo_union u2 = {.ptr = a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  union foo_union u3 = {.l = a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'long' in initializer list}}
}
