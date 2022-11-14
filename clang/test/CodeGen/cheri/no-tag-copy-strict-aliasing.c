// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -o - -O0 | FileCheck %s --check-prefixes=CHECK
// RUN: %riscv64_cheri_purecap_cc1 %s -relaxed-aliasing -emit-llvm -o - -O0 | FileCheck %s --check-prefixes=CHECK
/// Check that we don't add the no_preserve_cheri_tags attribute based on the
/// pointee type of the memcpy since that could break certain valid (albeit
/// dubious) code that relies on tag-preservation for types such as long*
/// See C2x 6.5p6 ("effective type") for more detail.

void *malloc(__SIZE_TYPE__);
void *memcpy(void *, const void *, __SIZE_TYPE__);
void foo(long **p, long **q);

void must_retain(long **p, long **q) {
  *p = malloc(32);
  *q = malloc(32);
  (*p)[0] = 1;
  (*p)[1] = 2;
  // Note: Despite the pointer being a long*, the C standard states that the
  // first store to a malloc'd memory location defines the type of the memory
  // for strict-aliasing purposes.
  // Therefore, this cast and store is fine and we need to retain tags
  // in the memcpy below (i.e. we can't add no_preserve_cheri_tags).
  *(void (**)(long **, long **))(*p + 2) = &foo;
  memcpy(*q, *p, 32);
  // CHECK: @must_retain(i64 addrspace(200)* addrspace(200)* [[P:%.*]], i64 addrspace(200)* addrspace(200)* [[Q:%.*]]) addrspace(200)
  // CHECK:    call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 {{%[0-9]+}}, i8 addrspace(200)* align 8 {{%[0-9]+}}
  // CHECK-SAME: , i64 32, i1 false){{$}}
  // CHECK-NEXT:    ret void
}

void no_retain(long **q) {
  long p[4];
  *q = malloc(32);
  p[0] = 1;
  p[1] = 2;
  *(void (**)(long **, long **))(p + 2) = &foo;
  memcpy(*q, p, 32);
  // Since we can see that p is a long[4] stack-allocated variable, the object
  // type is long[4] and therefore the memory should not be used to store tags.
  // We can therefore omit the copy of tags (although in practise it will
  // probably be aligned so malloc will retain tags at run time).
  // CHECK: @no_retain(i64 addrspace(200)* addrspace(200)* [[Q:%.*]]) addrspace(200)
  // CHECK:    call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 {{%[0-9]+}}, i8 addrspace(200)* align 8 {{%[0-9]+}}
  // CHECK-SAME: , i64 32, i1 false) [[NO_TAGS_ATTR:#.*]]{{$}}
  // CHECK-NEXT:    ret void
}

void retain_char_array(long **q) {
  // The C standard treats character arrays specially and therefore we must
  // assume that those can hold tags (even if they aren't sufficiently aligned
  // for holding capabilities).
  _Alignas(8) char p[32];
  *q = malloc(32);
  ((long *)p)[0] = 1;
  ((long *)p)[1] = 2;
  *(void (**)(long **, long **))(p + 16) = &foo;
  memcpy(*q, p, 32);
  // CHECK: @retain_char_array(i64 addrspace(200)* addrspace(200)* [[Q:%.*]]) addrspace(200)
  // CHECK:    call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 {{%[a-z0-9]+}}, i8 addrspace(200)* align 8 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 32, i1 false){{$}}
  // CHECK-NEXT:    ret void
}

// CHECK: [[NO_TAGS_ATTR]] = { no_preserve_cheri_tags }
