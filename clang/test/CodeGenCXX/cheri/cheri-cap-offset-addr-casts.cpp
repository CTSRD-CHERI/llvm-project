// RUN: %cheri_purecap_cc1 -std=c++11 -x c++ -emit-llvm -o - %s | FileCheck %s
//
// Test IR generation for __cheri_{offset, addr} when used with C++ features

class A {};
void f() { };

// Test object references
void objrefs(A& a) {
  long x1 = (__cheri_offset long)a;
  // CHECK: call i64 @llvm.cheri.cap.offset.get
  long x2 = (__cheri_addr long)a;
  // CHECK: call i64 @llvm.cheri.cap.address.get
}

// Test function references
void funcref(void (&f)(void)) {
  long x1 = (__cheri_offset long)f;
  // CHECK: call i64 @llvm.cheri.cap.offset.get
  long x2 = (__cheri_addr long)f;
  // CHECK: call i64 @llvm.cheri.cap.address.get
}
