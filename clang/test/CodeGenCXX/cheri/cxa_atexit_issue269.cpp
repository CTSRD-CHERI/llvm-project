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

// CHECK: @f = dso_local thread_local addrspace(200) global %struct.m zeroinitializer, align 1
// CHECK: @h = dso_local thread_local addrspace(200) global %struct.n zeroinitializer, align 1
// CHECK: @__tls_guard = internal thread_local unnamed_addr addrspace(200) global i1 false, align 1

void r() {
  // CHECK-LABEL: define dso_local void @_Z1rv()
  // CHECK: load i1, i1 addrspace(200)* @__tls_guard, align 1
  (void)f;
}
