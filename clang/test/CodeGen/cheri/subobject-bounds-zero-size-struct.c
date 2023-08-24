// RUN: %cheri_purecap_cc1 -std=gnu99 -cheri-bounds=subobject-safe -o - -emit-llvm -O0 %s | FileCheck %s
void do_stuff(void* arg);

struct {
} a;

// Setting bounds on empty structs was crashing
void test_global(void) { do_stuff(&a); }
// CHECK-LABEL: @test_global(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) @a, i64 0)

struct nested {
  struct {
  } a;
  int x;
};
void test_struct_ptr(struct nested* n) { do_stuff(&n->a); }
// CHECK-LABEL: @test_struct_ptr(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}}, i64 0)
void test_struct_byval(struct nested n) { do_stuff(&n.a); }
// CHECK-LABEL: @test_struct_byval(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}}, i64 0)
