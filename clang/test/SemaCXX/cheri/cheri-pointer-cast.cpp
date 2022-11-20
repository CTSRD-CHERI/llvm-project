// RUN: %clang_cc1 -triple mips64-unknown-freebsd -target-abi n64 %s -std=c++14 -verify=expected,nocheri,nohybrid,nopurecap
// RUN: %cheri_cc1  -target-abi n64 %s -std=c++14 -verify=expected,cheri,hybrid,nopurecap
// RUN: %cheri_purecap_cc1 %s -std=c++14 -verify=expected,cheri,nohybrid,purecap

#pragma clang diagnostic warning "-Wcapability-to-integer-cast"

typedef unsigned int uint;
typedef unsigned long ulong;

int foo;

#ifdef __CHERI__
#define CAP __capability
#else
#define CAP
#endif

#define DO_CASTS(dest, value)                             \
  do {                                                    \
    dest cstyle = (dest)value;                            \
    dest functional = dest(value);                        \
    dest staticCast = static_cast<dest>(value);           \
    dest reinterpretCast = reinterpret_cast<dest>(value); \
  } while (false)

int main() {
  void *CAP x = (int *CAP) & foo;
  DO_CASTS(ulong, x);
  // nohybrid-error@-1 {{static_cast from 'void *' to 'ulong' (aka 'unsigned long') is not allowed}}
  // hybrid-error@-2 {{static_cast from 'void * __capability' to 'ulong' (aka 'unsigned long') is not allowed}}
  // hybrid-warning@-3 3 {{cast from capability type 'void * __capability' to non-capability, non-address type 'ulong' (aka 'unsigned long') is most likely an error}}
  // purecap-warning@-4 3 {{cast from capability type 'void *' to non-capability, non-address type 'ulong' (aka 'unsigned long') is most likely an error}}
  DO_CASTS(uint, x);
  // nocheri-error@-1 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  // cheri-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  // nohybrid-error@-3 {{static_cast from 'void *' to 'uint' (aka 'unsigned int') is not allowed}}
  // hybrid-error@-4 {{static_cast from 'void * __capability' to 'uint' (aka 'unsigned int') is not allowed}}

  void *nocap = &foo;
  DO_CASTS(ulong, nocap); // expected-error {{static_cast from 'void *' to 'ulong' (aka 'unsigned long') is not allowed}}
  // purecap-warning@-1 3 {{cast from capability type 'void *' to non-capability, non-address type 'ulong' (aka 'unsigned long') is most likely an error}}

  DO_CASTS(uint, nocap);
  // nopurecap-error@-1 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  // purecap-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  // expected-error@-3 {{static_cast from 'void *' to 'uint' (aka 'unsigned int') is not allowed}}

  // also try casting from nullptr_t
  DO_CASTS(ulong, nullptr); //expected-error {{static_cast from 'std::nullptr_t' to 'ulong' (aka 'unsigned long') is not allowed}}
  DO_CASTS(uint, nullptr);
  // nopurecap-error@-1 3 {{cast from pointer to smaller type 'uint' (aka 'unsigned int') loses information}}
  // purecap-error@-2 3 {{cast from capability to smaller type 'uint' (aka 'unsigned int') loses information}}
  // expected-error@-3 {{static_cast from 'std::nullptr_t' to 'uint' (aka 'unsigned int') is not allowed}}

  // Check that static_cast from T* <-> T* __capability is not allowed
  typedef char *CAP char_capptr;
  char *cp = nullptr;
  DO_CASTS(char_capptr, cp);
  // hybrid-error@-1 {{static_cast from 'char *' to 'char_capptr' (aka 'char * __capability') changes capability qualifier}}

  typedef char *char_ptr;
  char *CAP cc = nullptr;
  DO_CASTS(char_ptr, cc);
  // hybrid-error@-1   {{static_cast from 'char * __capability' to 'char_ptr' (aka 'char *') changes capability qualifier}}
  // hybrid-error@-2 3 {{cast from capability type 'char * __capability' to non-capability type 'char_ptr' (aka 'char *') is most likely an error}}

  return 0;
}

// Check that we don't warn on dependent types
#ifdef __CHERI__
template <typename T>
long offset_get(T x) {
  return __builtin_cheri_offset_get(reinterpret_cast<void * __capability>(x));
}
template <typename T>
T offset_set(T x, long off) {
  return reinterpret_cast<T>(__builtin_cheri_offset_set(reinterpret_cast<void * __capability>(x), off));
  // hybrid-warning@-1 {{cast from capability type 'void * __capability' to non-capability, non-address type 'long'}}
  // purecap-warning@-2 {{cast from capability type 'void *' to non-capability, non-address type 'long'}}
  // purecap-warning@-3 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // hybrid-error@-4 {{cast from capability type 'void * __capability' to non-capability type 'x *'}}
}

struct x {
  short y;
  short z;
};

void use_offset_set() {
  long wrong = 0;
  long l = offset_set(wrong, 1); // expected-note {{in instantiation of function template specialization 'offset_set<long>' requested here}}
#ifndef __CHERI_PURE_CAPABILITY__
  struct x *wrong2 = nullptr;
  (void)offset_set(wrong2, 2); // expected-note {{in instantiation of function template specialization 'offset_set<x *>' requested here}}
#endif
  struct x *__capability correct = nullptr;
  (void)offset_set(correct, 2);
}
#endif
