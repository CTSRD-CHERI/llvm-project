// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O0 -emit-llvm %s
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O0 -emit-llvm %s
// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O0 -emit-llvm %s |  FileCheck %s -check-prefixes CHECK,PTR,SLOW,PTR-SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O2 -emit-llvm %s |  FileCheck %s -check-prefixes CHECK,PTR,OPT,PTR-OPT  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,LONG,SLOW,LONG-SLOW  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O2 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,LONG,OPT,LONG-OPT  -enable-var-scope

// TODO: tests for the CHERI case
#ifdef TEST_PTR
#define TYPE void *
#elif defined(TEST_LONG)
#define TYPE long
#else
#error MISSING TYPE
#endif

// Check that initializers are constant
// XXXAR: Note: they are evaluated by IntExprEvaluator and not CodeGen so work
// even if the Values emitted by CodeGen are wrong (which they initially were)
_Bool global_constant = __builtin_is_aligned(1024, 512);
_Bool global_constant2 = __builtin_is_p2aligned(1024, 11);
// SLOW: @global_constant = global i8 1, align 1
// SLOW: @global_constant2 = global i8 0, align 1
int a1 = __builtin_align_down(1023, 32);
int a2 = __builtin_align_up(1023, 32);
int a3 = __builtin_p2align_down(1023, 1);
int a4 = __builtin_p2align_up(1, 6);
// SLOW: @a1 = global i32 992, align 4
// SLOW: @a2 = global i32 1024, align 4
// SLOW: @a3 = global i32 1022, align 4
// SLOW: @a4 = global i32 64, align 4

TYPE get_type(void) {
  // CHECK: define{{( noalias)?}} [[$TYPE:.+]] @get_type()
  return (TYPE)0;
}

_Bool is_aligned(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // CHECK-LABEL: is_aligned(
  // PTR:      [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW:  [[VAR:%.+]] = load i64
  // SLOW: %set_bits = and i64 [[VAR]], 31
  // SLOW-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // SLOW-NEXT: ret i1 %is_aligned
  return __builtin_is_aligned(ptr, 32);
}

_Bool is_p2aligned(TYPE ptr, int p2align) {
  // CHECK-LABEL: is_p2aligned(
  // PTR:     [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW:  [[VAR:%.+]] = load i64
  // SLOW-NEXT: [[P2VAL:%.+]] = load i32, i32* %p2align.addr, align 4
  // SLOW-NEXT: %pow2 = zext i32 [[P2VAL]] to i64
  // SLOW-NEXT: %alignment = shl i64 1, %pow2
  // SLOW-NEXT: %mask = sub i64 %alignment, 1
  // SLOW-NEXT: %set_bits = and i64 [[VAR]], %mask
  // SLOW-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // SLOW-NEXT: ret i1 %is_aligned
  return __builtin_is_p2aligned(ptr, p2align);
}

TYPE align_up(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // CHECK-LABEL: @align_up(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      [[VAR2:%.+]] = add i64 [[VAR]], 31
  // SLOW:      [[MASKED:%.+]] = and i64 [[VAR2]], -32
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_align_up(ptr, 32);
}

TYPE p2align_up(TYPE ptr, unsigned p2align) {
  // CHECK-LABEL: @p2align_up(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      %alignment = shl i64 1, %pow2
  // SLOW:      %mask = sub i64 %alignment, 1
  // SLOW:      [[VAR_TMP:%.+]] = add i64 [[VAR]], %mask
  // SLOW:      %negated_mask = xor i64 %mask, -1
  // SLOW:      [[MASKED:%.+]] = and i64 [[VAR_TMP:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_p2align_up(ptr, p2align);
}

TYPE align_down(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // CHECK-LABEL: @align_down(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      [[MASKED:%.+]] = and i64 [[VAR]], -32
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_align_down(ptr, 32);
}

TYPE p2align_down(TYPE ptr, unsigned p2align) {
  // CHECK-LABEL: @p2align_down(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      %alignment = shl i64 1, %pow2
  // SLOW:      %mask = sub i64 %alignment, 1
  // SLOW:      %negated_mask = xor i64 %mask, -1
  // SLOW:      [[MASKED:%.+]] = and i64 [[VAR_TMP:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_p2align_down(ptr, p2align);
}

// Check that the inliner removes these constant calls at -O2 but not -O0:
_Bool inline_is_aligned(void) {
  // CHECK-LABEL: @inline_is_aligned(
  // SLOW: call zeroext i1 @is_aligned([[$TYPE]]{{.+}}100{{.*}}, i32 signext 32)
  // OPT: ret i1 false
  return is_aligned((TYPE)100, 32);
}

_Bool inline_is_p2aligned(void) {
  // CHECK-LABEL: @inline_is_p2aligned(
  // SLOW: call zeroext i1 @is_p2aligned([[$TYPE]]{{.+}}128{{.*}}, i32 signext 6)
  // OPT: ret i1 true
  return is_p2aligned((TYPE)128, 6);
}

TYPE inline_align_down(void) {
  // CHECK-LABEL: @inline_align_down(
  // SLOW: call [[$TYPE]] @align_down({{.+}}100{{.*}}, i32 signext 32)
  // LONG-OPT: ret i64 96
  return align_down((TYPE)100, 32);
}

TYPE inline_p2align_down(void) {
  // CHECK-LABEL: @inline_p2align_down(
  // SLOW: call [[$TYPE]] @p2align_down({{.+}}100{{.*}}, i32 signext 10)
  // LONG-OPT: ret i64 0
  return p2align_down((TYPE)100, 10);
}

TYPE inline_align_up(void) {
  // CHECK-LABEL: @inline_align_up(
  // SLOW: call [[$TYPE]] @align_up({{.+}}100{{.*}}, i32 signext 32)
  // LONG-OPT: ret i64 128
  return align_up((TYPE)100, 32);
}

TYPE inline_p2align_up(void) {
  // CHECK-LABEL: @inline_p2align_up(
  // SLOW: call [[$TYPE]] @p2align_up({{.+}}100{{.*}}, i32 signext 10)
  // LONG-OPT: ret i64 1024
  return p2align_up((TYPE)100, 10);
}

// XXX: original tests, can probably remove

// Check that some constants get folded for __is_aligned
_Bool opt_is_aligned(TYPE x) {
  // CHECK-LABEL: @opt_is_aligned(
  // LONG-OPT-SAME: i64 signext [[VAR:%.+]])
  // PTR:     [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW:  [[VAR:%.+]] = load i64
  // CHECK: %set_bits = and i64 [[VAR]], 127
  // CHECK-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // CHECK-NEXT: ret i1 %is_aligned
  return __builtin_is_p2aligned(x, 7);
}

_Bool opt_is_aligned1(TYPE x) {
  // CHECK-LABEL: @opt_is_aligned1(
  // SLOW: [[PHI:%.+]] = phi i1 [ false, %{{.+}} ], [ %{{.+}}, %{{.+}} ]
  // SLOW: ret i1 [[PHI]]
  // OPT:  ret i1 true
  return __builtin_is_p2aligned(x, 0) && __builtin_is_aligned(x, 1);
}

_Bool opt_is_aligned2() {
  // This expression gets folded even at -O0:
  // CHECK-LABEL: @opt_is_aligned2(
  // CHECK: ret i1 false
  return __builtin_is_p2aligned(7, 3);
}

_Bool opt_is_aligned3() {
  // This expression gets folded even at -O0:
  // CHECK-LABEL: @opt_is_aligned3(
  // CHECK: ret i1 true
  return __builtin_is_aligned(1024, 512);
}
