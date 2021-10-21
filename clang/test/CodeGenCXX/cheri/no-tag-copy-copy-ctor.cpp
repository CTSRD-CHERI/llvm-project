// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -o - | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 %s -relaxed-aliasing -emit-llvm -o -| FileCheck %s
/// Check that we add the no_preserve_tags/must_preserve_tags attribute to
/// C++ copy constructors.
/// TODO: we may need to special-case char[] fields

struct TestThreeLongsFinal final {
  long a;
  long b;
  long c;
  TestThreeLongsFinal(const TestThreeLongsFinal &) = default;
};

TestThreeLongsFinal test_copy_ctor_longs_final(const TestThreeLongsFinal &t) {
  // Since this type only contains a long[] (and is final) we don't need to preserve tags when copying
  return t;
  // CHECK-LABEL: @_Z26test_copy_ctor_longs_finalRK19TestThreeLongsFinal(
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: i8 addrspace(200)* align 8 {{%[0-9]+}}, i8 addrspace(200)* align 8 {{%[0-9]+}}
  // CHECK-SAME: , i64 24, i1 false) [[NO_TAGS_ATTR:#[0-9]+]]{{$}}
  // CHECK-NEXT: ret void
}

struct TestLongArray {
  long array[5];
  TestLongArray(const TestLongArray &) = default;
};

TestLongArray test_copy_ctor_long_array(const TestLongArray &t) {
  // Since this type only contains a long[] we don't need to preserve tags when copying
  return t;
  // CHECK-LABEL: @_Z25test_copy_ctor_long_arrayRK13TestLongArray(
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: i8 addrspace(200)* align 8 {{%[0-9]+}}, i8 addrspace(200)* align 8 {{%[0-9]+}}
  // CHECK-SAME: , i64 40, i1 false) [[NO_TAGS_ATTR:#[0-9]+]]
  // CHECK-NEXT: ret void
}

struct TestCharPtr {
  char *cap;
  long array[5];
  TestCharPtr(const TestCharPtr &) = default;
};

TestCharPtr test_copy_ctor_with_ptr(const TestCharPtr &t) {
  // Since this type only contains a char[] we must preserve tags when copying
  return t;
  // CHECK-LABEL: @_Z23test_copy_ctor_with_ptrRK11TestCharPtr(
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: i8 addrspace(200)* align 16 {{%[0-9]+}}, i8 addrspace(200)* align 16 {{%[0-9]+}}
  // CHECK-SAME: , i64 64, i1 false) [[MUST_PRESERVE_TAGS_ATTR:#[0-9]+]]
  // CHECK-NEXT: ret void
}

// TODO: we may need to conservatively assume that char[] could be used to hold tags?
struct TestCharArray {
  // XXX: should we assume this can be used to store tags even though it is underaligned?
  char array[32];
  TestCharArray(const TestCharArray &) = default;
};

TestCharArray test_copy_ctor_char_array(const TestCharArray &t) {
  return t;
  // CHECK-LABEL: @_Z25test_copy_ctor_char_arrayRK13TestCharArray(
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: i8 addrspace(200)* align 1 {{%[0-9]+}}, i8 addrspace(200)* align 1 {{%[0-9]+}}
  // CHECK-SAME: , i64 32, i1 false) [[NO_TAGS_ATTR]]
  // CHECK-NEXT: ret void
}

struct TestOveralignedCharArray {
  // NB: we have to assume this array can be used to store tags.
  alignas(32) char array[32];
  TestOveralignedCharArray(const TestOveralignedCharArray &) = default;
};

TestOveralignedCharArray test_copy_ctor_overaligned_char_array(const TestOveralignedCharArray &t) {
  return t;
  // CHECK-LABEL: @_Z37test_copy_ctor_overaligned_char_arrayRK24TestOveralignedCharArray(
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: i8 addrspace(200)* align 32 {{%[0-9]+}}, i8 addrspace(200)* align 32 {{%[0-9]+}}
  // FIXME: we should be conservative here and not set the attribute since there is
  //  an overaligned char[] that could potentially hold capabilities.
  // TODO-CHECK-SAME: , i64 32, i1 false){{$}}
  // CHECK-SAME: , i64 32, i1 false) [[NO_TAGS_ATTR]]
  // CHECK-NEXT: ret void
}

// CHECK: [[NO_TAGS_ATTR]] = { no_preserve_cheri_tags }
// CHECK: [[MUST_PRESERVE_TAGS_ATTR]] = { must_preserve_cheri_tags }
