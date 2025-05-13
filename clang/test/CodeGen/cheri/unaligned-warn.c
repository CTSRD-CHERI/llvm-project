// REQUIRES: mips-registered-target

// RUN: %cheri128_purecap_cc1 -O0 -o - -emit-llvm %s -w | FileCheck %s

// This diagnostic is disabled at -O0 -> must check at -O2
// RUN: %cheri128_purecap_cc1 -debug-info-kind=standalone -DNDEBUG -O2 -S -o /dev/null %s -verify

// If we are using -Werror this warning should not fail the build, only if it is explicitly added:
// RUN: %cheri128_purecap_cc1 -O2 -o /dev/null -S %s 2> /dev/null
// RUN: %cheri128_purecap_cc1 -O2 -o /dev/null -S %s 2>/dev/null

void * memcpy(void *, const void *, unsigned long);

typedef unsigned __intcap a;
void *b;

unsigned __intcap get_cap(void);

void test_dst_unliagned_src_cap_memcpy_warn(void* align1, short* align2, int* align4, long* align8, void** align_cap, a* src) {
  // CHECK-LABEL @test_dst_unliagned_src_cap_memcpy_warn(
  memcpy(align1, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka 'unsigned __intcap') and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to (u)intptr_t*}}
#ifndef NDEBUG
  // expected-note@-3{{You may want to recompile with -gline-tables-only}}
#endif
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200)
  // CHECK-SAME: align 1 %{{.+}}, ptr addrspace(200) align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A:#[0-9]+]]

  memcpy(align2, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka 'unsigned __intcap') and underaligned destination (aligned to 2 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to (u)intptr_t*}}
#ifndef NDEBUG
  // expected-note@-3{{You may want to recompile with -gline-tables-only}}
#endif
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200)
  // CHECK-SAME: align 2 %{{.+}}, ptr addrspace(200) align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align4, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka 'unsigned __intcap') and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to (u)intptr_t*}}
#ifndef NDEBUG
  // expected-note@-3{{You may want to recompile with -gline-tables-only}}
#endif
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200)
  // CHECK-SAME: align 4 %{{.+}}, ptr addrspace(200) align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align8, src, sizeof(*src));
  // expected-warning@-1{{memcpy operation with capability argument 'a' (aka 'unsigned __intcap') and underaligned destination (aligned to 8 bytes) may be inefficient or result in CHERI tags bits being stripped}}
  // expected-note@-2{{use __builtin_assume_aligned() or cast to (u)intptr_t*}}
#ifndef NDEBUG
  // expected-note@-3{{You may want to recompile with -gline-tables-only}}
#endif
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200)
  // CHECK-SAME: align 8 %{{.+}}, ptr addrspace(200) align 16 %{{.+}}, i64 16, i1 false)
  // CHECK-SAME: [[PRESERVE_TAGS_ATTRIB_TYPE_A]]
  memcpy(align_cap, src, sizeof(*src)); // this is fine!
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200)
  // CHECK-SAME: align 16 %{{.+}}, ptr addrspace(200) align 16 %{{.+}}, i64 16, i1 false)
}

