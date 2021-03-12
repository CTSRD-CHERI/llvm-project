// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -o - -O0 | FileCheck %s --check-prefixes=CHECK --implicit-check-not=llvm.memcpy
/// Check that we can add the no_preserve_cheri_tags attribute for copies
/// to/from globals (as those have a fixed effective type).

static __uint128_t u128 = 0;
extern __uint128_t *u128_ptr;
extern __uint128_t u128_array[4];
extern __uint128_t u128_array_unsized[];

void test_u128(void *buf) {
  // CHECK-LABEL: @test_u128(
  // No need to preserve tags since we can see the definition:
  __builtin_memcpy(buf, &u128, sizeof(u128));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 16, i1 false) [[NO_PRESERVE_TAGS:#[0-9]+]]{{$}}

  // We can see the variable declaration but it's a pointer so we can't add the attribute
  __builtin_memcpy(buf, u128_ptr, sizeof(*u128_ptr));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 16, i1 false){{$}}

  // Here we can see the array definition, so no need to preserve tags:
  __builtin_memcpy(buf, u128_array, sizeof(u128_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 64, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  /// TODO: For unsized global arrays we could assume that the copy stays in-bounds
  /// for the tag-preservation analysis, but for now we just don't set the attribute.
  __builtin_memcpy(buf, u128_array_unsized, 80);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 80, i1 false){{$}}
  // TODO-SAME: , i64 80, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
}

// CHECK: declare void @llvm.memcpy.p200i8.p200i8.i64(

static unsigned __intcap ucap = 0;
extern unsigned __intcap *ucap_ptr;
extern unsigned __intcap ucap_array[4];
extern unsigned __intcap ucap_array_unsized[];

void test_ucap(void *buf) {
  // CHECK-LABEL: @test_ucap(
  // In all of these cases we should add the must_preserve_tags attribute since
  // we are copying a capability type.
  __builtin_memcpy(buf, &ucap, sizeof(ucap));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_UINTCAP:#[0-9]+]]{{$}}
  __builtin_memcpy(buf, ucap_ptr, sizeof(*ucap_ptr));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_UINTCAP]]{{$}}
  __builtin_memcpy(buf, ucap_array, sizeof(ucap_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 64, i1 false) [[MUST_PRESERVE_UINTCAP]]{{$}}
  __builtin_memcpy(buf, ucap_array_unsized, 80);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 80, i1 false) [[MUST_PRESERVE_UINTCAP]]{{$}}
}

struct NoCaps {
  int i;
  long l[15];
};
extern struct NoCaps nocap_struct;
extern struct NoCaps *nocap_struct_ptr;
extern struct NoCaps nocap_struct_array[4];
extern struct NoCaps nocap_struct_array_unsized[];

void test_struct_nocaps(void *buf) {
  // CHECK-LABEL: @test_struct_nocaps(
  // No need to preserve tags since we can see the definition:
  __builtin_memcpy(buf, &nocap_struct, sizeof(nocap_struct));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 128, i1 false) [[NO_PRESERVE_TAGS]]{{$}}

  // We can see the variable declaration but it's a pointer so we can't add the attribute
  __builtin_memcpy(buf, nocap_struct_ptr, sizeof(*nocap_struct_ptr));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 128, i1 false){{$}}

  /// Here we can see the array definition, so no need to preserve tags:
  __builtin_memcpy(buf, nocap_struct_array, sizeof(nocap_struct_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 512, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
  /// TODO: For unsized global arrays we could assume that the copy stays in-bounds
  /// for the tag-preservation analysis, but for now we just don't set the attribute.
  __builtin_memcpy(buf, nocap_struct_array_unsized, sizeof(struct NoCaps) * 3);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 384, i1 false){{$}}
  // TODO-CHECK-SAME: , i64 384, i1 false) [[NO_PRESERVE_TAGS]]{{$}}
}

struct WithCaps {
  int i;
  union {
    long l;
    void *p;
  } u;
};
extern struct WithCaps withcap_struct;
extern struct WithCaps *withcap_struct_ptr;
extern struct WithCaps withcap_struct_array[4];
extern struct WithCaps withcap_struct_array_unsized[];

void test_struct_with_caps(void *buf) {
  // CHECK-LABEL: @test_struct_with_caps(
  // In all of these cases we should add the must_preserve_tags attribute since
  // we are copying a capability type.
  __builtin_memcpy(buf, &withcap_struct, sizeof(withcap_struct));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP:#[0-9]+]]{{$}}
  __builtin_memcpy(buf, withcap_struct_ptr, sizeof(*withcap_struct_ptr));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP]]{{$}}
  __builtin_memcpy(buf, withcap_struct_array, sizeof(withcap_struct_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 128, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP]]{{$}}
  __builtin_memcpy(buf, withcap_struct_array_unsized, 80);
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 80, i1 false) [[MUST_PRESERVE_STRUCT_WITHCAP]]{{$}}
}

extern char global_char_array[32];
extern unsigned char global_uchar_array[32];
extern signed char global_schar_array[32];

void test_char_array(void *buf) {
  // CHECK-LABEL: @test_char_array(
  // Arrays of type "(unsigned) char" are special and can be assumed to hold any type.
  // We extend this to signed char as well to avoid potentially surprising behaviour.
  // None of these cases should add the no_preserve_tags attribute:
  __builtin_memcpy(buf, &global_char_array, sizeof(global_char_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &global_uchar_array, sizeof(global_uchar_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
  __builtin_memcpy(buf, &global_schar_array, sizeof(global_schar_array));
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(
  // CHECK-SAME: , i64 32, i1 false){{$}}
}

// CHECK: [[NO_PRESERVE_TAGS]] = { no_preserve_cheri_tags }
// CHECK: [[MUST_PRESERVE_UINTCAP]] = { must_preserve_cheri_tags "frontend-memtransfer-type"="'unsigned __intcap'" }
// CHECK: [[MUST_PRESERVE_STRUCT_WITHCAP]] = { must_preserve_cheri_tags "frontend-memtransfer-type"="'struct WithCaps'" }
