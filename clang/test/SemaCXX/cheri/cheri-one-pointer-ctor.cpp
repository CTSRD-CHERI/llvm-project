// RUN: %cheri_cc1 -std=c++11 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -verify
// RUN: %cheri_purecap_cc1 -std=c++11 -x c++ -o - %s -fsyntax-only -Wall -Wno-unused-variable -std=c++11
// previous changes broke constructors that take one pointer argument and treated them as a functional C style case (-Wcapability-to-integer-cast)

#ifdef __CHERI_PURE_CAPABILITY__
// expected-no-diagnostics
#endif

template<class T>
class future {
public:
  typedef T(*Func)();
  future(Func f);
};

void test_func() {
  return;
}

future<void> test1() {
  return future<void>(test_func);
}
future<void> test2() {
  return future<void>(&test_func);  // this used to warn about casting a capability to future<void>
}

// Array initialization test case from libc++ memory.cpp:


#if __cplusplus >= 201103L
#define NULL nullptr
#define _LIBCPP_CONSTEXPR constexpr
#define _NOEXCEPT noexcept
#else
#define NULL __null
#define _LIBCPP_CONSTEXPR
#define _NOEXCEPT throw()
#endif

class __sp_mut {
  void* __lx;
public:
  void lock() _NOEXCEPT;
  void unlock() _NOEXCEPT;

private:
  _LIBCPP_CONSTEXPR __sp_mut(void*) _NOEXCEPT;
  __sp_mut(const __sp_mut&);
  __sp_mut& operator=(const __sp_mut&);

  friend __sp_mut& __get_sp_mut(const void*);
};

_LIBCPP_CONSTEXPR __sp_mut::__sp_mut(void* p) _NOEXCEPT
    : __lx(p)
{
}


struct pthread_mutex;
typedef pthread_mutex* pthread_mutex_t;
typedef pthread_mutex_t __libcpp_mutex_t;
#define _LIBCPP_MUTEX_INITIALIZER NULL


#define _LIBCPP_SAFE_STATIC __attribute__((__require_constant_initialization__))
_LIBCPP_SAFE_STATIC static const long __sp_mut_count = 4;
_LIBCPP_SAFE_STATIC static __libcpp_mutex_t mut_back[__sp_mut_count] =
{
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
};

__sp_mut&
__get_sp_mut(const void* p)
{
  // all these here are fine
  __sp_mut dummy{&mut_back[ 0]};
  __sp_mut dummy2 = {&mut_back[ 0]};
  __sp_mut dummy3 = &mut_back[ 0];
  static __sp_mut muts[__sp_mut_count]
  {
      &mut_back[ 0], &mut_back[ 1], &mut_back[ 2], &mut_back[ 3],
  };
  // return muts[hash<const void*>()(p) & (__sp_mut_count-1)];
  extern unsigned index;
  return muts[index & (__sp_mut_count-1)];
}

class ctor_with_cap_arg {
public:
  ctor_with_cap_arg(void* __capability);
};

void test_cap_conversion_still_fails() {
  static ctor_with_cap_arg muts[__sp_mut_count]
  {
      &mut_back[ 0], &mut_back[ 1], &mut_back[ 2], &mut_back[ 3],
  };
  static  pthread_mutex* __capability muts2[__sp_mut_count]
  {
      mut_back[ 0], mut_back[ 1], mut_back[ 2], mut_back[ 3]
  };
#ifndef __CHERI_PURE_CAPABILITY__
  // expected-warning@-3 4 {{converting non-capability type '__libcpp_mutex_t' (aka 'pthread_mutex *') to capability type 'pthread_mutex * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
#endif
}
