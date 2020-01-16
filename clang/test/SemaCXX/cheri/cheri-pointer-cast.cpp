// RUN: %clang_cc1 -triple mips64-unknown-freebsd -target-abi n64 %s -std=c++14 -verify
// RUN: %cheri_cc1  -target-abi n64 %s -std=c++14 -verify
// RUN: %cheri_purecap_cc1 %s -std=c++14 -verify

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"

typedef unsigned int uint;
typedef unsigned long ulong;

int foo;

#ifdef __CHERI__
#define CAP __capability
#else
#define CAP
#endif

#define DO_CASTS(dest, value) do { \
  dest cstyle = (dest)value; \
  dest functional = dest(value); \
  dest staticCast = static_cast<dest>(value); \
  dest reinterpretCast = reinterpret_cast<dest>(value); \
} while(false)

int main() {
  void* CAP x = (int* CAP)&foo;
  DO_CASTS(ulong, x); //expected-error-re {{static_cast from 'void *{{( __capability)?}}' to 'ulong' (aka 'unsigned long') is not allowed}}
#ifdef __CHERI__
  // expected-warning@-2 3 {{cast from capability type 'void * __capability' to non-capability, non-address type 'ulong' (aka 'unsigned long') is most likely an error}}
#endif
  DO_CASTS(uint, x);
#ifdef __CHERI__
  // expected-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-3 {{static_cast from 'void * __capability' to 'uint' (aka 'unsigned int') is not allowed}}
#else
  // expected-error@-5 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-6 {{static_cast from 'void *' to 'uint' (aka 'unsigned int') is not allowed}}
#endif

  void* nocap = &foo;
  DO_CASTS(ulong, nocap); //expected-error-re {{static_cast from 'void *{{( __capability)?}}' to 'ulong' (aka 'unsigned long') is not allowed}}
#ifdef __CHERI_PURE_CAPABILITY__
  // expected-warning@-2 3 {{cast from capability type 'void * __capability' to non-capability, non-address type 'ulong' (aka 'unsigned long') is most likely an error}}
#endif

  DO_CASTS(uint, nocap);
#ifdef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-3 {{static_cast from 'void * __capability' to 'uint' (aka 'unsigned int') is not allowed}}
#else
  // expected-error@-5 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-6 {{static_cast from 'void *' to 'uint' (aka 'unsigned int') is not allowed}}
#endif

  // also try casting from nullptr_t
  DO_CASTS(ulong, nullptr); //expected-error {{static_cast from 'nullptr_t' to 'ulong' (aka 'unsigned long') is not allowed}}
  DO_CASTS(uint, nullptr);
#ifdef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-3 {{static_cast from 'nullptr_t' to 'uint' (aka 'unsigned int') is not allowed}}
#else
  // expected-error@-5 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  //expected-error@-6 {{static_cast from 'nullptr_t' to 'uint' (aka 'unsigned int') is not allowed}}
#endif
  return 0;
}


// Check that we don't warn on dependent types
#ifdef __CHERI__
template<typename T>
long offset_get(T x) {
  return __builtin_cheri_offset_get(reinterpret_cast<void* CAP>(x));
}
template<typename T>
T offset_set(T x, long off) {
  return reinterpret_cast<T>(__builtin_cheri_offset_set(reinterpret_cast<void* CAP>(x), off));
  // expected-warning@-1 {{cast from capability type 'void * __capability' to non-capability, non-address type 'long'}}
  // expected-warning@-2 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}} expected-note@-2 {{insert cast to intptr_t to silence this warning}}
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-error@-4 {{cast from capability type 'void * __capability' to non-capability type 'x *'}}
#endif
}

struct x {
    short y;
    short z;
};

void use_offset_set() {
  long wrong = 0;
  long l = offset_set(wrong, 1); // expected-note {{in instantiation of function template specialization 'offset_set<long>' requested here}}
#ifndef __CHERI_PURE_CAPABILITY__
  struct x * wrong2 = nullptr;
  (void)offset_set(wrong2, 2); // expected-note {{in instantiation of function template specialization 'offset_set<x *>' requested here}}
#endif
  struct x * CAP correct = nullptr;
  (void)offset_set(correct, 2);
}
#endif
