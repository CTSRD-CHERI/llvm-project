// RUN: %riscv64_cheri_purecap_cc1 -xc %s -o - -emit-llvm -verify | FileCheck %s --check-prefixes=CHECK,CHECK-C --implicit-check-not=llvm.memcpy.
// RUN: %riscv64_cheri_purecap_cc1 -xc++ %s -o - -emit-llvm -verify | FileCheck %s --check-prefixes=CHECK,CHECK-CXX --implicit-check-not=llvm.memcpy.
// expected-no-diagnostics
/// Check that we look at the entire structure when deciding whether to add the
/// no_preserve_tags attribute for memcpy() calls on struct members: the memcpy
/// could extend past this field and copy adjacent instances of the same struct.

struct Nested {
  int i;
  long l;
  float f;
};

struct TestWithCap {
  char *ptr;
  __uint128_t not_a_cap;
  long array[4];
  struct Nested n;
  char *ptr2;
};

struct TestNoCap {
  __uint128_t not_a_cap;
  struct Nested n;
  long array[4];
  long pad;
};

void test_member_expr_byval(void *buf, struct TestWithCap t, struct TestNoCap t2) {
  // CHECK-C-LABEL: void @test_member_expr_byval(
  // CHECK-CXX-LABEL: void @_Z22test_member_expr_byvalPv11TestWithCap9TestNoCap(

  // For now we can't add the attribute for calls with member access despite the
  // address-of being on one of the non-cap fields, since we must conservatively
  // assume that the memcpy extends across all fields of the struct (and possibly
  // following instances of that struct for arrays).
  // Note: we could look at constant sizes if we wanted to perform a more
  // sophisticated analysis, but this is unlikely to be a big win for performance.
  __builtin_memcpy(buf, &t, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP:#[0-9]+]]{{$}}
  __builtin_memcpy(buf, &t.ptr, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_CHARPTR:#[0-9]+]]{{$}}
  // No attribute for the following four cases:
  __builtin_memcpy(buf, &t.not_a_cap, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, t.array, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &t.n, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, (&t)->array, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &(&t)->not_a_cap, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}

  // However, for the struct without capabilities all of these should be safe:
  // However, we don't know here that there is no subclass that could have
  // capabilities following the current object.
  // TODO: If we know the size of the copy <= sizeof(T), we should set the attribute.
  __builtin_memcpy(buf, &t2, sizeof(t2));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 80, i1 false) [[NO_PRESERVE_TAGS:#[0-9]+]]{{$}}
  /// The following call could copy trailing caps:
  __builtin_memcpy(buf, &t2, sizeof(t2) + sizeof(void *));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 96, i1 false){{$}}
  __builtin_memcpy(buf, &t2.not_a_cap, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, t2.array, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, &t2.n, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, (&t2)->array, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, &(&t2)->not_a_cap, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}

  // Direct assignment should always the no_preserve_tags attribute (the C2x
  // 6.5 memcpy+"Allocated objects have no declared type." case does not apply):
  t.n = t2.n;
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 24, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
}

// To avoid --implicit-check-not error:
// CHECK: declare void @llvm.memcpy.p200i8.p200i8.i64(

void test_member_expr_ptr(void *buf, struct TestWithCap *t, struct TestNoCap *t2) {
  // CHECK-C-LABEL: void @test_member_expr_ptr(
  // CHECK-CXX-LABEL: void @_Z20test_member_expr_ptrPvP11TestWithCapP9TestNoCap(

  // If the target VarDecl is a pointer, we can't assume that it matches the
  // underlying effective object type (C2x 6.5) so we have to be conservative
  // and not add the attribute (even for non-tag-bearing structs).
  // However, we do add the must_preserve_tags metadata calls where we know
  // that one of the types holds capabilities (as that is the most conservative
  // behaviour).
  __builtin_memcpy(buf, t, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP]]{{$}}
  __builtin_memcpy(buf, &t->ptr, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_CHARPTR]]{{$}}
  __builtin_memcpy(buf, &t->not_a_cap, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &t->array, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &t->n, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, (*t).array, 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &((*t).not_a_cap), 32);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}

  // We also can't add the attribute for the struct without caps since we don't
  // know the type of the underlying memory.
  __builtin_memcpy(buf, t2, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  __builtin_memcpy(buf, &t2->not_a_cap, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  __builtin_memcpy(buf, &t2->array, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  __builtin_memcpy(buf, &t2->n, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  __builtin_memcpy(buf, (*t2).array, 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}
  __builtin_memcpy(buf, &((*t2).not_a_cap), 40);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 40, i1 false){{$}}

  // Direct assignment should always the no_preserve_tags attribute (the C2x
  // 6.5 memcpy+"Allocated objects have no declared type." case does not apply):
  t->n = t2->n;
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 24, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
}

#ifdef __cplusplus
void test_member_expr_ref(void *buf, struct TestWithCap &t, struct TestNoCap &t2) {
  // CHECK-CXX-LABEL: void @_Z20test_member_expr_refPvR11TestWithCapR9TestNoCap(

  // For C++ references we can assume that those point to a valid object of
  // that type, since otherwise forming the reference would have been invalid.
  // For the member references the same caveats apply as in test_member_expr_byval()
  __builtin_memcpy(buf, &t, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP]]{{$}}
  __builtin_memcpy(buf, &t.ptr, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false) [[MUST_PRESERVE_CHARPTR]]{{$}}
  __builtin_memcpy(buf, &t.not_a_cap, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, t.array, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &t.n, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, (&t)->array, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &(&t)->not_a_cap, 32);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 32, i1 false){{$}}

  // However, for the struct without capabilities all of these should be safe:
  __builtin_memcpy(buf, &t2, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, &t2.not_a_cap, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, t2.array, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, &t2.n, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, (&t2)->array, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  __builtin_memcpy(buf, &(&t2)->not_a_cap, 40);
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 40, i1 false){{$}}
  // TODO-CHECK-CXX-SAME: , i64 40, i1 false) [[NO_PRESERVE_TAGS]]{{$}}

  // Direct assignment should always the no_preserve_tags attribute (the C2x
  // 6.5 memcpy+"Allocated objects have no declared type." case does not apply):
  t.n = t2.n;
  // CHECK-CXX: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-CXX-SAME: , i64 24, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
}
#endif

// CHECK: attributes [[MUST_PRESERVE_STRUCT_WITHCAP]] = { must_preserve_cheri_tags "frontend-memtransfer-type"="'struct TestWithCap'" }
// CHECK: attributes [[MUST_PRESERVE_CHARPTR]] = { must_preserve_cheri_tags "frontend-memtransfer-type"="'char * __capability'" }
// CHECK-C: attributes [[NO_PRESERVE_TAGS]] = { no_preserve_cheri_tags }
