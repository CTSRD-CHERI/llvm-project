// RUN: %cheri_cc1 -std=c++11 -target-abi purecap -fsyntax-only -verify %s
// RUN: %cheri_cc1 -std=c++11 -target-abi n64 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-freebsd -std=c++11 -fsyntax-only -verify %s

// expected-no-diagnostics
typedef decltype(nullptr) nullptr_t;
typedef decltype(__null) __null_t;

template<class T, class U>
struct is_same {
    enum { value = 0 };
};

template<class T>
struct is_same<T, T> {
    enum { value = 1 };
};

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null;
  void* y = nullptr;
  void* z = 0;
  nullptr_t nt1 = nullptr;
  nullptr_t nt2 = __null;
  nullptr_t nt3 = 0;
#ifdef __CHERI_PURE_CAPABILITY__
#define NULLPTR_SIZE (_MIPS_SZCAP / 8)
#define GNU_NULL_TYPE __intcap_t
#else
#define NULLPTR_SIZE 8
#define GNU_NULL_TYPE long
#endif
  static_assert(sizeof(__null_t) == NULLPTR_SIZE, "wrong NULL size");
  static_assert(sizeof(nullptr_t) == NULLPTR_SIZE, "wrong NULL size");
  static_assert(is_same<__null_t, GNU_NULL_TYPE>::value, "wrong NULL type");
  return !!x;
}
