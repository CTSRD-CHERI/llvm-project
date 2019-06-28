// REQUIRES: mips-registered-target

// RUN: %cheri128_purecap_cc1 -O0 -o - -emit-llvm %s -w | FileCheck %s
// RUN: %cheri128_purecap_cc1 -DBUILTIN  -O0 -o - -emit-llvm %s -w | FileCheck %s

// This diagnostic is disabled at -O0 -> must check at -O2
// RUN: %cheri128_purecap_cc1 -debug-info-kind=standalone -O2 -S -o /dev/null %s -verify
// RUN: %cheri128_purecap_cc1 -debug-info-kind=standalone -DBUILTIN -O2 -S -o /dev/null %s -verify

// If we are using -Werror this warning should not fail the build, only if it is explicitly added:
// RUN: %cheri128_purecap_cc1 -O2 -o /dev/null -S %s -Werror 2> /dev/null
// RUN: not %cheri128_purecap_cc1 -O2 -o /dev/null -S %s -Werror=cheri-misaligned 2>/dev/null

#ifdef BUILTIN
#define memcpy __builtin_memcpy
#define memmove __builtin_memmove
#else
void * memcpy(void *, const void *, unsigned long);
void * memmove(void *, const void *, unsigned long);
#endif

typedef __uintcap_t a;
void *b;

__uintcap_t get_cap(void);

void test_dst_unliagned_src_cap_memcpy(void* align1, short* align2, int* align4, long* align8, void** align_cap, a* src) {
  // CHECK-LABEL @test_dst_unliagned_src_cap_memcpy(
  memcpy(align1, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A:#[0-9]+]]

  memcpy(align2, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 2 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align4, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 4 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align8, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align_cap, src, sizeof(*src)); // this is fine!
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

struct without_cap {
  char a;
  int b;
};

struct with_cap {
  struct without_cap a;
  void* cap;
};

void test_no_warn_for_non_caps(short* align2, int not_a_cap, __uintcap_t* capptr,
                               struct with_cap* struct_with_cap, struct without_cap* struct_without_cap) {
  // CHECK-LABEL @test_no_warn_for_non_caps(

  memcpy(align2, &not_a_cap, sizeof(not_a_cap));  // no warning
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 4 %{{.+}}, i64 4, i1 false)

  memcpy(align2, struct_without_cap, sizeof(*struct_without_cap)); // no warning
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 4 %{{.+}}, i64 8, i1 false)

  memcpy(align2, capptr, sizeof(*capptr));
  // expected-warning@-1{{memcpy operation with capability argument '__uintcap_t' and underaligned destination (aligned to 2 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP:#[0-9]+]]

  memcpy(align2, struct_with_cap, sizeof(*struct_with_cap));
  // expected-warning@-1{{memcpy operation with capability argument 'struct with_cap' and underaligned destination (aligned to 2 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 32, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_STRUCT_WITH_CAP:#[0-9]+]]
}


void test_dst_unliagned_src_cap_memmove(void* align1, short* align2, int* align4, long* align8, void** align_cap, a* src) {
  // CHECK-LABEL @test_dst_unliagned_src_cap_memcpy(
  memmove(align1, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove(align2, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 2 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 2 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove(align4, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 4 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove(align8, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove(align_cap, src, sizeof(*src)); // this is fine!
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

#define memcpy_chk(x,y,z) __builtin___memcpy_chk(x,y,z, __builtin_object_size(x,0))
#define memcpy_chk_inbounds(x,y,z) __builtin___memcpy_chk(x,y,z, z)
#define memmove_chk(x,y,z) __builtin___memmove_chk(x,y,z, __builtin_object_size(x,0))
#define memmove_chk_inbounds(x,y,z) __builtin___memmove_chk(x,y,z, z)

void test_memcpy_chk(void* align1, long* align8, void** align_cap, a* src) {
  // CHECK-LABEL @test_memcpy_chk(
  memcpy_chk(align1, src, sizeof(*src));
  // expected-warning@-1{{__memcpy_chk operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(
  memcpy_chk(align8, src, sizeof(*src));
  // expected-warning@-1{{__memcpy_chk operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(

  memcpy_chk(align_cap, src, sizeof(*src)); // no warning
  // CHECK: call i8 addrspace(200)* @__memcpy_chk(

  // these are always turned into a memcpy:
  memcpy_chk_inbounds(align1, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy_chk_inbounds(align8, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy_chk_inbounds(align_cap, src, sizeof(*src));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

void test_memmove_chk(void* align1, long* align8, void** align_cap, a* src) {
  // CHECK-LABEL @test_memmove_chk(
  memmove_chk(align1, src, sizeof(*src));
  // expected-warning@-1{{__memmove_chk operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memmove_chk(
  memmove_chk(align8, src, sizeof(*src));
  // expected-warning@-1{{__memmove_chk operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call i8 addrspace(200)* @__memmove_chk(
  memmove_chk(align_cap, src, sizeof(*src)); // no warning
  // CHECK: call i8 addrspace(200)* @__memmove_chk(


  // these are always turned into a memmove:
  memmove_chk_inbounds(align1, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove_chk_inbounds(align8, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  memmove_chk_inbounds(align_cap, src, sizeof(*src)); // no warning
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

void test_builtin_assume_aligned_fix_1(long *align8, long *align8_again, char *align1, char *align1_again, __uintcap_t* src) {
  // CHECK-LABEL: @test_builtin_assume_aligned_fix_1(

  memcpy(align8, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument '__uintcap_t' and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]

  memmove(align8_again, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument '__uintcap_t' and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 8 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]


  memcpy(align1, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument '__uintcap_t' and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]

  memmove(align1_again, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument '__uintcap_t' and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]
}

void test_builtin_assume_aligned_fix_2(long *align8, long *align8_again, char *align1, char *align1_again, __uintcap_t* src) {
  // CHECK-LABEL: @test_builtin_assume_aligned_fix_2(
  // should not warn if we add __builtin_assume_aligned or cast to __intcap_t
  // But this only works at -O1 or higher:
  memcpy(__builtin_assume_aligned(align8, sizeof(void *)), src, sizeof(*src));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]
  memmove(__builtin_assume_aligned(align1, sizeof(void *)), src, sizeof(*src));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]
  memmove((__intcap_t *)align1_again, src, sizeof(*src));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 16 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
}

void test_builtin_assume_aligned_fix_3(long *align8, __uintcap_t* src) {
  // CHECK-LABEL: @test_builtin_assume_aligned_fix_3(
  // Check that we inferred align 8 in the warning here:
  memcpy(__builtin_assume_aligned(align8, sizeof(long)), src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument '__uintcap_t' and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]

  memmove(__builtin_assume_aligned(align8, sizeof(long)), src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument '__uintcap_t' and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]
}


extern __uintcap_t foo_array[10][2];
void do_stuff(char* buf);
void test_no_crash_with_array(void) {
  // CHECK-LABEL: @test_no_crash_with_array(
  char buffer[1234];
  memcpy(buffer, foo_array, sizeof(foo_array));
  // Since the merge, these are now turned into an intrinsic in the front end
  // so this warning is never seen.  They are then turned into a memcpy call in
  // the back end, which doesn't warn.
  // not-expected-warning@-1{{memcpy operation with capability argument '__uintcap_t' and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // not-expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 {{.+}}, i64 320, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]]
  do_stuff(buffer); // So that buffer is not optimized away
}

void test_builtin_assume_aligned_intermediate_var(char *align1, char *align1_again, a* src) {
  // CHECK-LABEL: @test_builtin_assume_aligned_intermediate_var(
  // this should still warn:
  void *align4 = __builtin_assume_aligned(align1, 4);
  memcpy(align4, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  void *align32 = __builtin_assume_aligned(align1_again, 32);
  memcpy(align32, src, sizeof(*src)); // this is fine
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
}

void test_builtin_assume_aligned_memmove_intermediate_var(char *align1, char *align1_again, a* src) {
  // CHECK-LABEL: @test_builtin_assume_aligned_memmove_intermediate_var(
  // this should still warn:
  void *align4 = __builtin_assume_aligned(align1, 4);
  memmove(align4, src, sizeof(*src));
  // expected-warning@-1{{memmove operation with capability argument 'a' (aka '__uintcap_t') and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to __intcap_t*}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]

  void *align32 = __builtin_assume_aligned(align1_again, 32);
  memmove(align32, src, sizeof(*src)); // this is fine
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)*
  // CHECK-SAME: align 1 %{{.+}}, i8 addrspace(200)* align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
}

// CHECK: attributes [[PRESERVE_TAGS_ATTRIB_TYPE_A]] = { "frontend-memtransfer-type"="'a' (aka '__uintcap_t')" "must-preserve-cheri-tags" }
// CHECK: attributes [[PRESERVE_TAGS_ATTRIB_TYPE_UINTCAP]] = { "frontend-memtransfer-type"="'__uintcap_t'" "must-preserve-cheri-tags" }
// CHECK: attributes [[PRESERVE_TAGS_ATTRIB_TYPE_STRUCT_WITH_CAP]] = { "frontend-memtransfer-type"="'struct with_cap'" "must-preserve-cheri-tags" }
