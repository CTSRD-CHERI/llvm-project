// RUN: %cheri_purecap_cc1 -O2 -std=c++11 -fcxx-exceptions -fexceptions -mllvm -cheri-cap-table-abi=pcrel -o -  %s -emit-llvm | FileCheck %s
// Check that this doesn't crash during codegen
// RUN: %cheri_purecap_cc1 -O2 -std=c++11 -fcxx-exceptions -fexceptions -mllvm -cheri-cap-table-abi=pcrel -o /dev/null  %s -S

// __tls_guard was being emitted in AS200 which caused selection failures later
// See https://github.com/CTSRD-CHERI/llvm/issues/269

struct m {};
struct n {
  n() {}
};
thread_local m f;
thread_local n h;

// CHECK: @f = thread_local global %struct.m zeroinitializer, align 1
// CHECK: @h = thread_local global %struct.n zeroinitializer, align 1
// CHECK: @__tls_guard = internal thread_local unnamed_addr global i1 false, align 1

void r() {
  // CHECK-LABEL: define void @_Z1rv()
  // CHECK: load i1, i1* @__tls_guard, align 1
  (void)f;
}
