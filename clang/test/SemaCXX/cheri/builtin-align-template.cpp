// RUN: %cheri_cc1 -std=c++11 -o - %s -fsyntax-only -verify
// RUN: %cheri_purecap_cc1 -std=c++11 -o - %s -fsyntax-only -verify
// Check that we don't crash in codegen:
// RUN: %cheri_purecap_cc1 -std=c++11 -o - %s -emit-llvm -DCODEGEN | llvm-cxxfilt | FileCheck %s


// Check that we don't crash when using dependent types:
// https://github.com/CTSRD-CHERI/llvm-project/issues/330
template <typename a, a b> void* c(void* d) { // expected-note{{candidate template ignored}}
  return __builtin_align_down(d, b);
}

struct x {};
x foo;
// instantiate it:
void test(void* value) {
  c<int, 16>(value);
  // CHECK: call i8 addrspace(200)* @void* cap c<int, 16>(void*)(i8 addrspace(200)*
#ifndef CODEGEN
  c<struct x, foo>(value); // expected-error{{no matching function for call to 'c'}}
#endif
}


// The fix for the issue above broke some legitimate code. Here is a regression test:
typedef __SIZE_TYPE__ size_t;
void* allocate_impl(size_t size);
template<typename T>
T* allocate() {
       constexpr size_t allocation_size =
           __builtin_align_up(sizeof(T), sizeof(void*));
       return static_cast<T*>(
          __builtin_assume_aligned(allocate_impl(allocation_size), sizeof(void*)));
}
struct Foo {
  int value;
};
void* test2() {
  return allocate<struct Foo>();
}
