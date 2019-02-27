// RUN: %cheri_purecap_cc1 -O0 -o - -emit-llvm %s -verify | FileCheck %s
// RUN: %cheri_purecap_cc1 -DBUILTIN -O0 -o - -emit-llvm %s -verify | FileCheck %s


typedef __uintcap_t a;
void *b;

#ifdef BUILTIN
#define memcpy __builtin_memcpy
#define memmove __builtin_memmove
#else
void * memcpy(void *, const void *, unsigned long);
void * memmove(void *, const void *, unsigned long);
#endif

void test_dst_unliagned_src_cap_memcpy(void* align1, short* align2, int* align4, long* align8, void** align_cap) {
  // CHECK-LABEL @test_dst_unliagned_src_cap_memcpy(
  a local_cap = 0;

  memcpy(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memcpy(align2, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 2 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)


  memcpy(align4, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 4 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 4 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memcpy(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memcpy(align_cap, &local_cap, sizeof(local_cap)); // this is fine!
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}


void test_dst_unliagned_src_cap_memmove(void* align1, short* align2, int* align4, long* align8, void** align_cap) {
  // CHECK-LABEL @test_dst_unliagned_src_cap_memcpy(
  a local_cap = 0;

  memmove(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memmove(align2, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 2 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)


  memmove(align4, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 4 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 4 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memmove(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memmove(align_cap, &local_cap, sizeof(local_cap)); // this is fine!
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

#define memcpy_chk(x,y,z) __builtin___memcpy_chk(x,y,z, __builtin_object_size(x,0))
#define memcpy_chk_inbounds(x,y,z) __builtin___memcpy_chk(x,y,z, z)
#define memmove_chk(x,y,z) __builtin___memmove_chk(x,y,z, __builtin_object_size(x,0))
#define memmove_chk_inbounds(x,y,z) __builtin___memmove_chk(x,y,z, z)

void test_memcpy_chk(void* align1, long* align8, void** align_cap) {
  // CHECK-LABEL @test_memcpy_chk(
  a local_cap = 0;
  memcpy_chk(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{__memcpy_chk operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(
  memcpy_chk(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{__memcpy_chk operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(

  memcpy_chk(align_cap, &local_cap, sizeof(local_cap)); // no warning
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(

  // these are always turned into a memcpy:
  memcpy_chk_inbounds(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  memcpy_chk_inbounds(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memcpy operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  memcpy_chk_inbounds(align_cap, &local_cap, sizeof(local_cap));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

}

void test_memmove_chk(void* align1, long* align8, void** align_cap) {
  // CHECK-LABEL @test_memmove_chk(
  a local_cap = 0;
  memmove_chk(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{__memmove_chk operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memmove_chk(
  memmove_chk(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{__memmove_chk operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memmove_chk(
  memmove_chk(align_cap, &local_cap, sizeof(local_cap)); // no warning
  // CHECK: call i8 addrspace(200)* @__memmove_chk(


  // these are always turned into a memmove:
  memmove_chk_inbounds(align1, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 1 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  memmove_chk_inbounds(align8, &local_cap, sizeof(local_cap));
  // expected-warning@-1{{memmove operation with underaligned capability argument (aligned to 8 bytes) may result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

  memmove_chk_inbounds(align_cap, &local_cap, sizeof(local_cap)); // no warning
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)

}
