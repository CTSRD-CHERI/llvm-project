// RUN: %cheri_cc1 -std=c11 -o - %s -fsyntax-only -verify
// RUN: %cheri_purecap_cc1 -std=c11 -o - %s -fsyntax-only -verify

#define NULL ((void*)0)

// These warnings only happen in the hybrid ABI
#ifndef __CHERI_PURE_CAPABILITY__

struct foo {
  void* __capability cap;
  void* ptr;
};

void test_cap_to_ptr(void* __capability a) {
  void* non_cap = a; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
  struct foo f = {a, a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  struct foo f2;
  f2 = (struct foo){a, NULL}; // this is fine
  f2 = (struct foo){a, a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  struct foo f3 = {NULL, NULL};

  // C99 designated initializer:
  struct foo f4 = { .cap = a, .ptr = NULL};
  struct foo f5 = { .cap = a, .ptr = a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
}

// check arrays:
void test_arrays(void* __capability cap) {
  int* ptr = NULL;
  void* ptr_array[3] = { NULL, cap, ptr }; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  // GCC range init:
  void* ptr_array2[3] = { [0 ... 1] = NULL, cap }; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  void* ptr_array3[3] = { [0 ... 2] = cap }; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  // TODO: this should probably warn about pointer -> cap conversion

  void* __capability cap_array[3] = { NULL, cap, ptr };  // expected-error {{converting non-capability type 'int *' to capability type 'void * __capability' without an explicit cast}}

  struct foo foo_array[5] = {
      {cap, NULL}, // no-error
      {cap, cap}, // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
      {.cap = NULL, .ptr = NULL},
      [4] = {.cap = cap, .ptr = cap} // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  };
}

union foo_union {
  void* ptr;
  long l;
};

void test_union(void* __capability a) {
  union foo_union u = {a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  u = (union foo_union){a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  union foo_union u2 = {.ptr = a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  union foo_union u3 = {.l = a}; // expected-warning {{incompatible pointer to integer conversion initializing 'long' with an expression of type 'void * __capability'}}
}

#else /* __CHERI_PURE_CAPABILITY__ */
// allow the conversion from void * -> bool as a special case
// expected-no-diagnostics

typedef _Bool bool;
#define true 1
#define false 0

struct pointer_and_bool {
  const char* name;
  bool value;
};

struct pointer_and_bool pb_array[3] = {
    { "foo", false },
    { "bar", true },
    { NULL, NULL },
};

#endif
