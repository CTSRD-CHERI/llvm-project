// RUN: %cheri_cc1 -o - -O2 -S %s -verify
// See https://github.com/CTSRD-CHERI/llvm/issues/271
// This is just to show how broken variadics are in they hybrid ABI

extern void call_cap(void *__capability);
extern void call_int(int i);
extern void call_va_list(__builtin_va_list v);

int __attribute__((noinline)) expect_cap_as_arg1(void *a, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, a);
  void * __capability cp = __builtin_va_arg(ap, void * __capability);
  int x = __builtin_va_arg(ap, int);
  int y = __builtin_va_arg(ap, int);
  call_cap(cp);
  call_int(x);
  call_int(y);
  // printf("test 7=%td 8=%d p=%tx 9=%d\n", (ptrdiff_t)a, x, (ptrdiff_t)(__cheri_fromcap void *)cp, y);
  __builtin_va_end(ap);
  return 0;
}

int __attribute__((noinline)) use_va_copy(void *a, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, a);
  __builtin_va_list ap2;
  __builtin_va_copy(ap2, ap);
  call_va_list(ap2);
  // printf("test 7=%td 8=%d p=%tx 9=%d\n", (ptrdiff_t)a, x, (ptrdiff_t)(__cheri_fromcap void *)cp, y);
  __builtin_va_end(ap);
  return 0;
}

int foo(void* __capability cap1) {
  expect_cap_as_arg1((void*)0, cap1, 1, 2); // expected-error{{It is not possible to pass capabilities to variadic functions in the hybrid ABI due to limitations in the MIPS ABI}}
  use_va_copy((void*)0, cap1, 1, 2); // expected-error{{It is not possible to pass capabilities to variadic functions in the hybrid ABI due to limitations in the MIPS ABI}}
}

