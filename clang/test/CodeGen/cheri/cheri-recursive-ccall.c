// Previously compiling a recursive CCall would break the TBAA verifier because
// it was emitting a load of the whole struct instead of the individual elements
// RUN: %cheri_purecap_cc1 -no-struct-path-tbaa -emit-llvm -O2 -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -new-struct-path-tbaa -emit-llvm -O2 -o - %s | FileCheck %s
struct {
  void *a;
  void *b;
} c;
__attribute__((cheri_ccall)) __attribute__((cheri_method_class(c))) int
d(void) {
  d();
// CHECK-LABEL: define chericcallcc i32 @d()
// CHECK:      [[NUM:%.+]] = load i64, i64{{( addrspace\(200\))?}}* @__cheri_method.c.d, align 8, !invariant.load
// CHECK-NEXT: [[ARG1:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* getelementptr inbounds (%struct.anon, %struct.anon addrspace(200)* @c, i64 0, i32 0), align
// CHECK-NEXT: [[ARG2:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* getelementptr inbounds (%struct.anon, %struct.anon addrspace(200)* @c, i64 0, i32 1), align
// CHECK-NEXT: tail call chericcallcc i32 @cheri_invoke(i8 addrspace(200)* [[ARG1]], i8 addrspace(200)* [[ARG2]], i64 zeroext [[NUM]])
// CHECK-NEXT: ret i32 0
  return 0;
}
