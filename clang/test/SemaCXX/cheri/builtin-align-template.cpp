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
