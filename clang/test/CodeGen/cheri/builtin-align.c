// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O0 -emit-llvm %s     | FileCheck %s -check-prefixes COMMON,CHECK,NOCAP-IR,PTR,SLOW,PTR-SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O0 -emit-llvm %s    | FileCheck %s -check-prefixes COMMON,CHECK,NOCAP-IR,LONG,SLOW,LONG-SLOW  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_CAP -Wno-tautological-compare -o - -O0 -emit-llvm %s     | FileCheck %s -check-prefixes COMMON,CHECK,CAP,SLOW,CAP-SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_UINTCAP -Wno-tautological-compare -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes COMMON,CHECK,CAP,SLOW,CAP-SLOW -enable-var-scope

// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O2 -emit-llvm %s    | FileCheck %s -check-prefixes  COMMON,OPT,PTR-OPT -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O2 -emit-llvm %s   | FileCheck %s -check-prefixes  COMMON,OPT,LONG-OPT  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_CAP -Wno-tautological-compare -o - -O2 -emit-llvm %s    | FileCheck %s -check-prefixes  COMMON,OPT,CAP-OPT,CAPPTR-OPT -enable-var-scope
// RUN: %cheri_cc1 -DTEST_UINTCAP -Wno-tautological-compare -o - -O2 -emit-llvm %s | FileCheck %s -check-prefixes COMMON,OPT,CAP-OPT,UINTCAP-OPT -enable-var-scope

void *add(void *arg, __PTRDIFF_TYPE__ diff) {
  return (char *)arg + diff;
}
// TODO: tests for the CHERI case
#ifdef TEST_PTR
#define TYPE void *
#elif defined(TEST_LONG)
#define TYPE long
#elif defined(TEST_CAP)
#define TYPE void *__capability
#elif defined(TEST_UINTCAP)
#define TYPE __uintcap_t
#else
#error MISSING TYPE
#endif

// Check that initializers are constant
// XXXAR: Note: they are evaluated by IntExprEvaluator and not CodeGen so work
// even if the Values emitted by CodeGen are wrong (which they initially were)
_Bool global_constant = __builtin_is_aligned(1024, 512);
// SLOW: @global_constant = global i8 1, align 1
int a1 = __builtin_align_down(1023, 32);
int a2 = __builtin_align_up(1023, 32);
// SLOW: @a1 = global i32 992, align 4
// SLOW: @a2 = global i32 1024, align 4

TYPE get_type(void) {
  // CHECK: define{{( noalias)?}} [[$TYPE:.+]] @get_type()
  return (TYPE)0;
}

_Bool is_aligned(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // COMMON-LABEL: is_aligned(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* {{%.+}})
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      %alignment = zext i32 {{.+}} to i64
  // SLOW-NEXT: %mask = sub i64 %alignment, 1
  // SLOW-NEXT: %set_bits = and i64 [[VAR]], %mask
  // SLOW-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // SLOW-NEXT: ret i1 %is_aligned
  return __builtin_is_aligned(ptr, align);
}

TYPE align_up(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // COMMON-LABEL: @align_up(
  // align up/align down is different for capabilities and pointers since we can't just mask
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* [[SRC:%.+]])
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG:      [[VAR:%.+]] = load i64
  // CHECK:      %alignment = zext i32 %{{.+}} to i64
  // CHECK-NEXT: %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // CHECK-NEXT: [[VAR_TMP:%.+]] = add i64 [[VAR]], %mask
  // CHECK-NEXT: %negated_mask = xor i64 %mask, -1
  // CHECK-NEXT: [[MASKED:%.+]] = and i64 [[VAR_TMP:%.+]], %negated_mask
  // CAP-NEXT:   [[RESULT:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* [[SRC]], i64 [[MASKED]])
  // CAP-NEXT:   ret i8 addrspace(200)* [[RESULT:%.+]]
  // PTR-NEXT:   %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR-NEXT:   ret [[$TYPE]] %aligned_result
  // LONG-SLOW:  ret i64 [[MASKED]]
  return __builtin_align_up(ptr, align);
}

TYPE align_down(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // COMMON-LABEL: @align_down(
  // CAP:        [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* [[SRC:%.+]])
  // PTR:        [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG:       [[VAR:%.+]] = load i64
  // CHECK:      %alignment = zext i32 %{{.+}} to i64
  // CHECK-NEXT: %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // CHECK-NEXT: %negated_mask = xor i64 %mask, -1
  // CHECK-NEXT: [[MASKED:%.+]] = and i64 [[VAR:%.+]], %negated_mask
  // CAP-NEXT:   [[RESULT:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* [[SRC]], i64 [[MASKED]])
  // CAP-NEXT:   ret i8 addrspace(200)* [[RESULT:%.+]]
  // PTR-NEXT:   %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR-NEXT:   ret [[$TYPE]] %aligned_result
  // LONG-NEXT:  ret i64 [[MASKED]]
  return __builtin_align_down(ptr, align);
}

// Check that the inliner removes these constant calls at -O2 but not -O0:
_Bool inline_is_aligned(void) {
  // COMMON-LABEL: @inline_is_aligned(
  // SLOW: call zeroext i1 @is_aligned([[$TYPE]]{{.+(100|%[0-9]).*}}, i32 signext 32)
  // OPT: ret i1 false
  return is_aligned((TYPE)100, 32);
}

TYPE inline_align_down(void) {
  // COMMON-LABEL: @inline_align_down(
  // SLOW: call [[$TYPE]] @align_down({{.+(100|%[0-9]).*}}, i32 signext 32)
  // LONG-OPT: ret i64 96
  // PTR-OPT: ret i8* inttoptr (i64 96 to i8*)
  // CAPPTR-OPT: call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* nonnull inttoptr (i64 100 to i8 addrspace(200)*), i64 96)
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 96)
  return align_down((TYPE)100, 32);
}

TYPE inline_align_up(void) {
  // COMMON-LABEL: @inline_align_up(
  // SLOW: call [[$TYPE]] @align_up({{.+(100|%[0-9]).*}}, i32 signext 32)
  // LONG-OPT: ret i64 128
  // PTR-OPT: ret i8* inttoptr (i64 128 to i8*)
  // CAPPTR-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* nonnull inttoptr (i64 100 to i8 addrspace(200)*), i64 128)
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 128)
  return align_up((TYPE)100, 32);
}

// The following used to assert (second parameter wider than the first)
// See https://github.com/CTSRD-CHERI/clang/issues/183
int alignment_wider_than_source(int a, int b) {
  // COMMON-LABEL: @alignment_wider_than_source(
  // CHECK: [[UP:%.+]] = add i32 %{{.+}}, 3
  // CHECK: [[RESULT:%.+]] = and i32 [[UP:%.+]], -4
  // CHECK: ret i32 [[RESULT]]
  return __builtin_align_up(a, sizeof(b));
}

_Bool constant_is_aligned() {
  // Check that this expression gets folded even at -O0:
  // COMMON-LABEL: @opt_is_aligned3(
  // CHECK: ret i1 true
  return __builtin_is_aligned(1024, 512);
}
