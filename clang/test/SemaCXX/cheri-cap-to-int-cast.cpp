// RUN: %clang_cc1 -triple cheri-unknown-freebsd -std=c++11 -o - %s -fsyntax-only -verify
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -std=c++11 -target-abi purecap -o - %s -fsyntax-only -verify

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"


#if !__has_attribute(memory_address)
#error "memory_address attribute not supported"
#endif

#if !__has_extension(__cheri_cast)
#error "__cheri_cast feature should exist"
#endif

void* __capability a;
using vaddr_t = __attribute__((memory_address)) unsigned __PTRDIFF_TYPE__;
using double_attribute = __attribute__((memory_address)) vaddr_t;  // expected-warning {{attribute 'memory_address' is already applied}}
using err_cap_type = __attribute__((memory_address)) __intcap_t;  // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses ('__intcap_t' is invalid)}}
using err_struct_type = __attribute__((memory_address)) struct foo; // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses ('struct foo' is invalid)}}
using err_pointer_type = int* __attribute__((memory_address)); // expected-error {{'memory_address' attribute only applies to integer types that can store memory addresses}}

typedef const vaddr_t other_addr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __uintcap_t uintptr_t;
typedef __intcap_t intptr_t;
typedef uintptr_t word;
struct test {
  int x;
};

void cast_vaddr() {
  // These casts should be fine, typedef with attribute
  vaddr_t v = reinterpret_cast<vaddr_t>(a);
  v = static_cast<vaddr_t>(a); // expected-error {{static_cast from 'void * __capability' to 'vaddr_t' (aka 'unsigned long') is not allowed}}
  v = (vaddr_t)a;
  v = vaddr_t(a);
  v = vaddr_t{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'vaddr_t' (aka 'unsigned long') in initializer list}}
}

void cast_vaddr2() {
  // These casts should be fine, typedef with attribute
  long __attribute__((memory_address)) v = reinterpret_cast<long __attribute__((memory_address))>(a);
  v = static_cast<long __attribute__((memory_address))>(a); // expected-error {{static_cast from 'void * __capability' to 'long __attribute__((memory_address))' is not allowed}}
  v = (long __attribute__((memory_address)))a;
  // these won't work, need single token for functional/brace-init:
  // v = long __attribute__((memory_address))(a);
  // v = long __attribute__((memory_address)){a};

  __attribute__((memory_address)) long v2 = reinterpret_cast<__attribute__((memory_address)) long>(a);
  v2 = static_cast<__attribute__((memory_address)) long>(a); // expected-error {{static_cast from 'void * __capability' to 'long __attribute__((memory_address))' is not allowed}}
  v2 = (__attribute__((memory_address)) long)a;
}

void cast_long() {
  long v = reinterpret_cast<long>(a); // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  v = static_cast<long>(a);  // expected-error {{static_cast from 'void * __capability' to 'long' is not allowed}}
  v = (long)a;  // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  v = long(a); // expected-warning {{cast from capability type 'void * __capability' to non-capability, non-address type 'long' is most likely an error}}
  v = long{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'long' in initializer list}}
}


// Should cause narrowing errors
void cast_int() {
  int v = reinterpret_cast<int>(a); // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = static_cast<int>(a);  // expected-error {{static_cast from 'void * __capability' to 'int' is not allowed}}
  v = (int)a; // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = int(a); // expected-error {{cast from capability to smaller type 'int' loses information}}
  v = int{a}; // expected-error {{type 'void * __capability' cannot be narrowed to 'int' in initializer list}}
}

void cast_uintcap() {
  __uintcap_t v = reinterpret_cast<__uintcap_t>(a);
  // XXXAR: should we allow this static cast?
  v = static_cast<__uintcap_t>(a); // expected-error {{static_cast from 'void * __capability' to '__uintcap_t' is not allowed}}
  v = (__uintcap_t)a;
  v = __uintcap_t(a);
  v = __uintcap_t{a}; // expected-error {{cannot initialize a value of type '__uintcap_t' with an lvalue of type 'void * __capability'}}
}

void cast_to_cap(void) {
  // noops that shouldn't warn
  auto p1 = reinterpret_cast<void* __capability>(a); // no warning
  auto p2 = (void* __capability)a; // no warning

  auto w1 = reinterpret_cast<word* __capability>(a); // no warning
  auto w2 = (word* __capability)a; // no warning
}


// Currently none of these warn: https://github.com/CTSRD-CHERI/clang/issues/140

void probably_an_error(__uintcap_t* cap) {
  long l = *cap;
  l += 1;
  *cap = l;
}

void check_uintcap_to_int() {
  __uintcap_t cap;
  int i = cap;
  i = (int)cap;
  i = int(cap);
  i = int{cap};  // expected-error {{type '__uintcap_t' cannot be narrowed to 'int' in initializer list}}
  i = static_cast<int>(cap);
  long l = cap;
  l = (long)cap;
  l = long(cap);
  l = long{cap}; // expected-error {{type '__uintcap_t' cannot be narrowed to 'long' in initializer list}}
  l = static_cast<long>(cap);

  i = reinterpret_cast<int>(cap);  // expected-error {{reinterpret_cast from '__uintcap_t' to 'int' is not allowed}}
  l = reinterpret_cast<long>(cap); // expected-error {{reinterpret_cast from '__uintcap_t' to 'long' is not allowed}}
}


class test_class {};

#define DO_ALL_CASTS(to, from) do { \
  to var = reinterpret_cast<to>(from); \
  var = static_cast<to>(from); \
  var = const_cast<to>(from); \
  var = dynamic_cast<to>(from); \
  var = (to)from; \
  var = to(from); \
  var = to{from}; \
} while(false)

void cast_ptr() {
  // XXXAR: these should also warn
  using voidp = void*;
  DO_ALL_CASTS(voidp, a);
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 5 {{cast from capability type 'void * __capability' to non-capability type 'voidp' (aka 'void *') is most likely an error}}
  // expected-note@-3 5{{use __cheri_cast to convert between pointers and capabilities}}
  // expected-error@-4 {{type 'void * __capability' cannot be narrowed to 'voidp' (aka 'void *') in initializer list}}
#endif
  // expected-error@-6 {{'void' is not a class}}


  using wordp = word*;
  DO_ALL_CASTS(wordp, a);
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 4 {{cast from capability type 'void * __capability' to non-capability type 'wordp' (aka '__uintcap_t *') is most likely an error}}
  // expected-note@-3 4 {{use __cheri_cast to convert between pointers and capabilities}}
  // expected-error@-4 {{type 'void * __capability' cannot be narrowed to 'wordp' (aka '__uintcap_t *') in initializer list}}
  // expected-error@-5 {{const_cast from 'void * __capability' to 'wordp' (aka '__uintcap_t *') is not allowed}}
#else
  // expected-error@-7 {{const_cast from 'void * __capability' to 'wordp' (aka '__uintcap_t * __capability') is not allowed}}
  // expected-error@-8 {{cannot initialize a value of type 'wordp' (aka '__uintcap_t * __capability') with an lvalue of type 'void * __capability'}}
#endif
  // expected-error@-10 {{'__uintcap_t' is not a class}}



  using test_class_ptr = test_class*;
  test_class* __capability b = nullptr;
  DO_ALL_CASTS(test_class_ptr, b);
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 5 {{cast from capability type 'test_class * __capability' to non-capability type 'test_class_ptr' (aka 'test_class *') is most likely an error}}
  // expected-error@-3 {{test_class * __capability' cannot be narrowed to 'test_class_ptr' (aka 'test_class *') in initializer list}}
  // expected-error@-4 {{static_cast from 'test_class * __capability' to 'test_class_ptr' (aka 'test_class *'), which are not related by inheritance, is not allowed}} // TODO: this should be a better error message
#endif
}

void cast_ref(int & __capability cap_ref) {
  int x;
  using intref = int&;
  int& v = reinterpret_cast<int&>(cap_ref); // expected-error{{cast from capability type 'int & __capability' to non-capability type 'int &' is most likely an error}} expected-note {{use __cheri_cast to convert between pointers and capabilities}}
  v = static_cast<int&>(cap_ref); // expected-error{{cast from capability type 'int & __capability' to non-capability type 'int &' is most likely an error}} expected-note {{use __cheri_cast to convert between pointers and capabilities}}
  // FIXME: this error shouldn/t be there twice:
  // expected-error@-2 {{converting capability type 'int & __capability' to non-capability type 'int &' without an explicit cast}}
  v = const_cast<int&>(cap_ref); // expected-error{{cast from capability type 'int & __capability' to non-capability type 'int &' is most likely an error}} expected-note {{use __cheri_cast to convert between pointers and capabilities}}
  v = dynamic_cast<int&>(cap_ref); // expected-error{{'int' is not a class}}
  v = (int&)cap_ref; // expected-error{{cast from capability type 'int & __capability' to non-capability type 'int &' is most likely an error}} expected-note {{use __cheri_cast to convert between pointers and capabilities}}
  v = intref(cap_ref); // expected-error{{cast from capability type 'int & __capability' to non-capability type 'intref' (aka 'int &') is most likely an error}} expected-note {{use __cheri_cast to convert between pointers and capabilities}}
  v = intref{cap_ref}; // expected-error{{converting capability type 'int & __capability' to non-capability type 'intref' (aka 'int &') without an explicit cast}}
}
