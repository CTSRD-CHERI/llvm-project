// RUN: %cheri_purecap_cc1 -O2 -std=c++11 -fcxx-exceptions -fexceptions -o -  %s -emit-llvm | FileCheck %s
/// Check that __tls_guard is emitted in AS200. This previously caused selection
/// failures (see https://github.com/CTSRD-CHERI/llvm/issues/269) but was fixed
/// by adding real TLS support, so keep checking that we emit it in AS200.

struct m {};
struct n {
  n() {}
};
thread_local m f;
thread_local n h;

// CHECK: @f = thread_local addrspace(200) global %struct.m zeroinitializer, align 1
// CHECK: @h = thread_local addrspace(200) global %struct.n zeroinitializer, align 1
// CHECK: @__tls_guard = internal thread_local unnamed_addr addrspace(200) global i1 false, align 1

void r() {
  // CHECK-LABEL: define dso_local void @_Z1rv()
  // CHECK: load i1, ptr addrspace(200) @__tls_guard, align 1
  (void)f;
}
