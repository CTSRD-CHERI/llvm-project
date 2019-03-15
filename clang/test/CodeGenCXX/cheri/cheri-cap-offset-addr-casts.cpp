// RUN: %cheri_purecap_cc1 -std=c++11 -x c++ -emit-llvm -o - %s | FileCheck %s
// Check that we can use the
// RUN: %cheri_cc1 -std=c++11 -fsyntax-only %s -verify=hybrid
//
// Test IR generation for __cheri_{offset, addr} when used with C++ features

class A {};
void f() { };

// Test object references
void objrefs(A& a) {
  long x1 = (__cheri_offset long)a; // hybrid-error{{invalid source type 'A &' for __cheri_offset: source must be a capability}}
  // CHECK: call i64 @llvm.cheri.cap.offset.get.i64
  long x2 = (__cheri_addr long)a;
  // CHECK: call i64 @llvm.cheri.cap.address.get.i64
}

// Test function references
void funcref(void (&f)(void)) {
  long x1 = (__cheri_offset long)f; // hybrid-error{{invalid source type 'void (*)()' for __cheri_offset: source must be a capability}}
  // CHECK: call i64 @llvm.cheri.cap.offset.get.i64
  long x2 = (__cheri_addr long)f;
  // CHECK: call i64 @llvm.cheri.cap.address.get.i64
}
