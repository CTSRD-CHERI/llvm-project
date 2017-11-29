// RUN: %cheri_cc1 -std=c++11 -o - %s -fsyntax-only -verify
// RUN: %cheri_cc1 -std=c++11 -target-abi purecap -o - %s -fsyntax-only -verify

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

using vaddr_t = __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__;

struct test {
  int x;
};

void test_capptr_to_int(void* __capability a) {
  vaddr_t v{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'vaddr_t' (aka 'unsigned long') in initializer list}}
  v = vaddr_t{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'vaddr_t' (aka 'unsigned long') in initializer list}}
  vaddr_t v2 = 0; v2 = {a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'vaddr_t' (aka 'unsigned long') in initializer list}}

  __uintcap_t uc{a}; // expected-error {{cannot initialize a variable of type '__uintcap_t' with an lvalue of type 'void * __capability'}}
  uc = __uintcap_t{a};  // expected-error {{cannot initialize a value of type '__uintcap_t' with an lvalue of type 'void * __capability'}}
  __uintcap_t uc2 = 0; uc2 = {a}; // expected-error {{cannot initialize a value of type '__uintcap_t' with an lvalue of type 'void * __capability'}}

  __intcap_t ic{a}; // expected-error {{cannot initialize a variable of type '__intcap_t' with an lvalue of type 'void * __capability'}}
  ic = __intcap_t{a};  // expected-error {{cannot initialize a value of type '__intcap_t' with an lvalue of type 'void * __capability'}}
  __intcap_t ic2 = 0; ic2 = {a}; // expected-error {{cannot initialize a value of type '__intcap_t' with an lvalue of type 'void * __capability'}}

  long l{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'long' in initializer list}}
  l = long{a};  // expected-error {{type 'void * __capability' cannot be narrowed to 'long' in initializer list}}
  long l2 = 0; l2 = {a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'long' in initializer list}}

  int i{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'int' in initializer list}}
  i = int{a};  // expected-error {{type 'void * __capability' cannot be narrowed to 'int' in initializer list}}
  int i2 = 0; i2 = {a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'int' in initializer list}}
}

void test_uintcap_to_int(__uintcap_t a) {
  vaddr_t v{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'vaddr_t'}}
  v = vaddr_t{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'vaddr_t'}}
  vaddr_t v2 = 0; v2 = {a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'vaddr_t'}}

  long l{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'long'}}
  l = long{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'long'}}
  long l2 = 0; l2 = {a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'long'}}

  int i{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'int'}}
  i = int{a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'int'}}
  int i2 = 0; i2 = {a}; // expected-error {{'__uintcap_t' cannot be narrowed to 'int'}}

  __uintcap_t uc{a};
  uc = __uintcap_t{a};
  __uintcap_t uc2 = 0; uc2 = {a};

  __intcap_t ic{a}; // expected-error {{non-constant-expression cannot be narrowed from type '__uintcap_t' to '__intcap_t'}} expected-note {{insert an explicit cast to silence this issue}}
  ic = __intcap_t{a}; // expected-error {{non-constant-expression cannot be narrowed from type '__uintcap_t' to '__intcap_t'}} expected-note {{insert an explicit cast to silence this issue}}
  __intcap_t ic2 = 0; ic2 = {a}; // expected-error {{non-constant-expression cannot be narrowed from type '__uintcap_t' to '__intcap_t'}} expected-note {{insert an explicit cast to silence this issue}}

}

// These warnings only happen in the hybrid ABI
#ifndef __CHERI_PURE_CAPABILITY__

struct foo {
  void* __capability cap;
  void* ptr;
};

void test_cap_to_ptr(void* __capability a) {
  void* non_cap{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  void* non_cap2 = {a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'void *' in initializer list}}
  // TODO: These should warn as well
  foo f{a, a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
  foo f2;
  f2 = {a, nullptr}; // this is fine
  // TODO: it would be nice if we could get a better error message here (see SemaOverload.cpp), it works fine for references
  f2 = {a, a}; // expected-error {{no viable overloaded '='}}
  // expected-note@-14 {{candidate function (the implicit copy assignment operator) not viable: cannot convert initializer list argument to 'const foo'}}
  // expected-note@-15 {{candidate function (the implicit move assignment operator) not viable: cannot convert initializer list argument to 'foo'}}
  foo f3{nullptr, nullptr};
}


// check arrays:
void test_arrays(void* __capability cap) {
  int* ptr = nullptr;
  void* ptr_array[3] = { nullptr, cap, ptr }; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
  void* __capability cap_array[3] = { nullptr, cap, ptr }; // expected-error {{converting non-capability type 'int *' to capability type 'void * __capability' without an explicit cast}}

    struct foo foo_array[5] = {
      {cap, nullptr}, // no-error
      {cap, cap}, // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
      {.cap = nullptr, .ptr = nullptr}, // no-error
      [4] = {.cap = cap, .ptr = cap} // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
  };
}

union foo_union {
  void* ptr;
  long l;
};

void test_union(void* __capability a) {
  foo_union u{a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
}

#endif
