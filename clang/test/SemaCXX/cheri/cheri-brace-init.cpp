// RUN: %cheri_cc1 -std=c++11 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_cc1 -std=c++11 -target-abi purecap -o - %s -fsyntax-only -verify=expected,purecap

#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

using ptraddr_t = __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__;

struct test {
  int x;
};

void test_capptr_to_int(void* __capability a) {
  ptraddr_t v{a};
  // hybrid-error@-1 {{cannot initialize a variable of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a variable of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void *'}}
  v = ptraddr_t{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void *'}}
  ptraddr_t v2 = 0; v2 = {a};
  // hybrid-error@-1 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'ptraddr_t' (aka 'unsigned long') with an lvalue of type 'void *'}}

  // NB: Compound literals are a GNU C++ extension so we need a single word alias
  using __uintcap = unsigned __intcap;
  __uintcap uc{a};
  // hybrid-error@-1 {{cannot initialize a variable of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a variable of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void *'}}
  uc = __uintcap{a};
  // hybrid-error@-1 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void *'}}
  __uintcap uc2 = 0; uc2 = {a};
  // hybrid-error@-1 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__uintcap' (aka 'unsigned __intcap') with an lvalue of type 'void *'}}

  __intcap ic{a};
  // hybrid-error@-1 {{cannot initialize a variable of type '__intcap' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a variable of type '__intcap' with an lvalue of type 'void *'}}
  ic = __intcap{a};
  // hybrid-error@-1 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void *'}}
  __intcap ic2 = 0; ic2 = {a};
  // hybrid-error@-1 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type '__intcap' with an lvalue of type 'void *'}}

  long l{a};
  // hybrid-error@-1 {{cannot initialize a variable of type 'long' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a variable of type 'long' with an lvalue of type 'void *'}}
  l = long{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'long' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'long' with an lvalue of type 'void *'}}
  long l2 = 0; l2 = {a};
  // hybrid-error@-1 {{cannot initialize a value of type 'long' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'long' with an lvalue of type 'void *'}}

  int i{a};
  // hybrid-error@-1 {{cannot initialize a variable of type 'int' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a variable of type 'int' with an lvalue of type 'void *'}}
  i = int{a};
  // hybrid-error@-1 {{cannot initialize a value of type 'int' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'int' with an lvalue of type 'void *'}}
  int i2 = 0; i2 = {a};
  // hybrid-error@-1 {{cannot initialize a value of type 'int' with an lvalue of type 'void * __capability'}}
  // purecap-error@-2 {{cannot initialize a value of type 'int' with an lvalue of type 'void *'}}
}

void test_uintcap_to_int(unsigned __intcap a) {
  ptraddr_t v{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'ptraddr_t' (aka 'unsigned long') in initializer list}} \
                // expected-note {{insert an explicit cast to silence this issue}}
  v = ptraddr_t{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'ptraddr_t' (aka 'unsigned long') in initializer list}} \
                  // expected-note {{insert an explicit cast to silence this issue}}
  ptraddr_t v2 = 0; v2 = {a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'ptraddr_t' (aka 'unsigned long') in initializer list}} \
                            // expected-note {{insert an explicit cast to silence this issue}}

  long l{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'long' in initializer list}} \
             // expected-note {{insert an explicit cast to silence this issue}}
  l = long{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'long' in initializer list}} \
               // expected-note {{insert an explicit cast to silence this issue}}
  long l2 = 0; l2 = {a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'long' in initializer list}} \
                         // expected-note {{insert an explicit cast to silence this issue}}

  int i{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'int' in initializer list}} \
            // expected-note {{insert an explicit cast to silence this issue}}
  i = int{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'int' in initializer list}} \
              // expected-note {{insert an explicit cast to silence this issue}}
  int i2 = 0; i2 = {a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to 'int' in initializer list}} \
                        // expected-note {{insert an explicit cast to silence this issue}}

  using __uintcap = unsigned __intcap;
  __uintcap uc{a};
  uc = __uintcap{a};
  __uintcap uc2 = 0; uc2 = {a};

  __intcap ic{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to '__intcap'}} expected-note {{insert an explicit cast to silence this issue}}
  ic = __intcap{a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to '__intcap'}} expected-note {{insert an explicit cast to silence this issue}}
  __intcap ic2 = 0; ic2 = {a}; // expected-error {{non-constant-expression cannot be narrowed from type 'unsigned __intcap' to '__intcap'}} expected-note {{insert an explicit cast to silence this issue}}

}

// These warnings only happen in the hybrid ABI
#ifndef __CHERI_PURE_CAPABILITY__

struct foo {
  void* __capability cap;
  void* ptr;
};

void test_cap_to_ptr(void* __capability a) {
  void* non_cap{a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
  void* non_cap2 = {a}; // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast; if this is intended use __cheri_fromcap}}
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

  struct foo foo_array1[3] = {
    {cap, nullptr}, // no-error
    {cap, cap}, // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
    {.cap = nullptr, .ptr = nullptr} // no-error
  };
  struct foo foo_array2[2] = {
    [1] = {.cap = cap, .ptr = cap} // expected-error {{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
    // expected-warning@-1 {{array designators are a C99 extension}}
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
