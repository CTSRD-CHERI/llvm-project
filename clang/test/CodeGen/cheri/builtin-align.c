// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O0 -emit-llvm %s |  FileCheck %s -check-prefixes CHECK,NOCAP-IR,PTR,SLOW,PTR-SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_PTR -Wno-tautological-compare -o - -O2 -emit-llvm %s |  FileCheck %s -check-prefixes CHECK,PTR,OPT,PTR-OPT  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,NOCAP-IR,LONG,SLOW,LONG-SLOW  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_LONG -Wno-tautological-compare -o - -O2 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,LONG,OPT,LONG-OPT  -enable-var-scope

// RUN: %cheri_cc1 -DTEST_CAP -Wno-tautological-compare -o - -O0 -emit-llvm %s     | FileCheck %s -check-prefixes CHECK,CAP,SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_CAP -Wno-tautological-compare -o - -O2 -emit-llvm %s     | FileCheck %s -check-prefixes CHECK,CAP,OPT,CAP-OPT  -enable-var-scope
// RUN: %cheri_cc1 -DTEST_UINTCAP -Wno-tautological-compare -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,CAP,SLOW -enable-var-scope
// RUN: %cheri_cc1 -DTEST_UINTCAP -Wno-tautological-compare -o - -O2 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,CAP,OPT,UINTCAP-OPT  -enable-var-scope

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
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* {{%.+}})
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // SLOW:      %alignment = zext i32 {{.+}} to i64
  // SLOW-NEXT: %mask = sub i64 %alignment, 1
  // SLOW-NEXT: %set_bits = and i64 [[VAR]], %mask
  // SLOW-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // SLOW-NEXT: ret i1 %is_aligned
  return __builtin_is_aligned(ptr, align);
}

_Bool is_p2aligned(TYPE ptr, int p2align) {
  // CHECK-LABEL: is_p2aligned(
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* {{%.+}})
  // LONG-SLOW: [[VAR:%.+]] = load i64
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
  // align up/align down is different for capabilities and pointers since we can't just mask
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[SRC:%.+]])
  // CAP:       %alignment = zext i32 %{{.+}} to i64
  // CAP-NEXT:  %mask = {{(sub i64 %alignment, 1)|(add nsw i64 %alignment, -1)}}
  // CAP-NEXT:  %unaligned_bits = and i64 [[VAR]], %mask
  // CAP-NEXT:  %is_aligned = icmp eq i64 %unaligned_bits, 0
  // CAP-NEXT:  %missing_bits = sub {{(nsw )?}}i64 %alignment, %unaligned_bits
  // CAP-NEXT:  %aligned_cap = getelementptr inbounds i8, i8 addrspace(200)* [[SRC]], i64 %missing_bits
  // CAP-NEXT:  %result = select i1 %is_aligned, i8 addrspace(200)* [[SRC]], i8 addrspace(200)* %aligned_cap

  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // NOCAP-IR:  %alignment = zext i32 %{{.+}} to i64
  // NOCAP-IR:  %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // NOCAP-IR:  [[VAR_TMP:%.+]] = add i64 [[VAR]], %mask
  // NOCAP-IR:  %negated_mask = xor i64 %mask, -1
  // NOCAP-IR:  [[MASKED:%.+]] = and i64 [[VAR_TMP:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_align_up(ptr, align);
}

TYPE p2align_up(TYPE ptr, unsigned p2align) {
  // CHECK-LABEL: @p2align_up(
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[SRC:%.+]])
  // CAP-SLOW:  %alignment = shl i64 1, %pow2
  // CAP:       %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // CAP-NEXT:  %unaligned_bits = and i64 [[VAR]], %mask
  // CAP-NEXT:  %is_aligned = icmp eq i64 %unaligned_bits, 0
  // CAP-NEXT:  %missing_bits = sub {{(nsw )?}}i64 %alignment, %unaligned_bits
  // CAP-NEXT:  %aligned_cap = getelementptr inbounds i8, i8 addrspace(200)* [[SRC]], i64 %missing_bits
  // CAP-NEXT:  %result = select i1 %is_aligned, i8 addrspace(200)* [[SRC]], i8 addrspace(200)* %aligned_cap

  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // NOCAP-IR:  %alignment = shl i64 1, %pow2
  // NOCAP-IR:  %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // NOCAP-IR:  [[VAR_TMP:%.+]] = add i64 [[VAR]], %mask
  // NOCAP-IR:  %negated_mask = xor i64 %mask, -1
  // NOCAP-IR:  [[MASKED:%.+]] = and i64 [[VAR_TMP:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_p2align_up(ptr, p2align);
}

TYPE align_down(TYPE ptr, unsigned align) {
  // TODO: should we allow non-constant values and just say not passing a power-of-two is undefined?
  // CHECK-LABEL: @align_down(
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[SRC:%.+]])
  // CAP:       %alignment = zext i32 %{{.+}} to i64
  // CAP-NEXT:  %mask = {{(sub i64 %alignment, 1)|(add nsw i64 %alignment, -1)}}
  // CAP-NEXT:  %unaligned_bits = and i64 [[VAR]], %mask
  // CAP-NEXT:  %sub = sub i64 0, %unaligned_bits
  // CAP-NEXT:  %aligned_cap = getelementptr inbounds i8, i8 addrspace(200)* [[SRC]], i64 %sub
  // CAP-NEXT:  ret i8 addrspace(200)* %aligned_cap

  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64
  // NOCAP-IR:  %alignment = zext i32 %{{.+}} to i64
  // NOCAP-IR:  %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // NOCAP-IR:  %negated_mask = xor i64 %mask, -1
  // NOCAP-IR:  [[MASKED:%.+]] = and i64 [[VAR:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_align_down(ptr, align);
}

TYPE p2align_down(TYPE ptr, unsigned p2align) {
  // CHECK-LABEL: @p2align_down(
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[SRC:%.+]])
  // PTR:       [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // LONG-SLOW: [[VAR:%.+]] = load i64


  // SLOW:      %alignment = shl i64 1, %pow2
  // CAP-OPT:   %notmask = shl nsw i64 -1, %pow2
  // CAP-SLOW:  %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // CAP-OPT:   %mask = xor i64 %notmask, -1
  // CAP:       %unaligned_bits = and i64 [[VAR]], %mask
  // CAP-NEXT:  %sub = sub i64 0, %unaligned_bits
  // CAP-NEXT:  %aligned_cap = getelementptr inbounds i8, i8 addrspace(200)* [[SRC]], i64 %sub
  // CAP-NEXT:  ret i8 addrspace(200)* %aligned_cap

  // NOCAP-IR:  %mask = {{(sub i64 %alignment, 1)|(add i64 %alignment, -1)}}
  // NOCAP-IR:  %negated_mask = xor i64 %mask, -1
  // NOCAP-IR:  [[MASKED:%.+]] = and i64 [[VAR:%.+]], %negated_mask
  // PTR:       %aligned_result = inttoptr i64 [[MASKED:%.+]] to i8*
  // PTR:       ret [[$TYPE]] %aligned_result
  // LONG-SLOW: ret i64 [[MASKED]]
  return __builtin_p2align_down(ptr, p2align);
}

// Check that the inliner removes these constant calls at -O2 but not -O0:
_Bool inline_is_aligned(void) {
  // CHECK-LABEL: @inline_is_aligned(
  // SLOW: call zeroext i1 @is_aligned([[$TYPE]]{{.+(100|%[0-9]).*}}, i32 signext 32)
  // OPT: ret i1 false
  return is_aligned((TYPE)100, 32);
}

_Bool inline_is_p2aligned(void) {
  // CHECK-LABEL: @inline_is_p2aligned(
  // SLOW: call zeroext i1 @is_p2aligned([[$TYPE]]{{.+(128|%[0-9]).*}}, i32 signext 6)
  // OPT: ret i1 true
  return is_p2aligned((TYPE)128, 6);
}

TYPE inline_align_down(void) {
  // CHECK-LABEL: @inline_align_down(
  // SLOW: call [[$TYPE]] @align_down({{.+(100|%[0-9]).*}}, i32 signext 32)
  // LONG-OPT: ret i64 96
  // PTR-OPT: ret i8* inttoptr (i64 96 to i8*)
  // CAP-OPT: ret i8 addrspace(200)* inttoptr (i64 96 to i8 addrspace(200)*)
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 96)
  return align_down((TYPE)100, 32);
}

TYPE inline_p2align_down(void) {
  // CHECK-LABEL: @inline_p2align_down(
  // SLOW: call [[$TYPE]] @p2align_down({{.+(100|%[0-9]).*}}, i32 signext 10)
  // LONG-OPT: ret i64 0
  // PTR-OPT: ret i8* null
  // CAP-OPT: ret i8 addrspace(200)* null
  // FIXME: this should be optimized to return null:
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 0)
  return p2align_down((TYPE)100, 10);
}

TYPE inline_align_up(void) {
  // CHECK-LABEL: @inline_align_up(
  // SLOW: call [[$TYPE]] @align_up({{.+(100|%[0-9]).*}}, i32 signext 32)
  // LONG-OPT: ret i64 128
  // PTR-OPT: ret i8* inttoptr (i64 128 to i8*)
  // CAP-OPT: ret i8 addrspace(200)* inttoptr (i64 128 to i8 addrspace(200)*)
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 128)
  return align_up((TYPE)100, 32);
}

TYPE inline_p2align_up(void) {
  // CHECK-LABEL: @inline_p2align_up(
  // SLOW: call [[$TYPE]] @p2align_up({{.+(100|%[0-9]).*}}, i32 signext 10)
  // LONG-OPT: ret i64 1024
  // PTR-OPT: ret i8* inttoptr (i64 1024 to i8*)
  // CAP-OPT: ret i8 addrspace(200)* inttoptr (i64 1024 to i8 addrspace(200)*)
  // UINTCAP-OPT: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 1024)
  return p2align_up((TYPE)100, 10);
}



// The following used to assert (second parameter wider than the first)
// See https://github.com/CTSRD-CHERI/clang/issues/183
int alignment_wider_than_source(int a, int b) {
  // CHECK-LABEL: @alignment_wider_than_source(
  // CHECK: [[UP:%.+]] = add i32 %{{.+}}, 3
  // CHECK: [[RESULT:%.+]] = and i32 [[UP:%.+]], -4
  // CHECK: ret i32 [[RESULT]]
  return __builtin_align_up(a, sizeof(b));
}

// XXX: original tests, can probably remove

// Check that some constants get folded for __is_aligned
_Bool opt_is_aligned(TYPE x) {
  // CHECK-LABEL: @opt_is_aligned(
  // LONG-OPT-SAME: i64 signext [[VAR:%.+]])
  // PTR:        [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // CAP:       [[VAR:%.+]] = {{(tail )?}}call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[SRC:%.+]])
  // LONG-SLOW:  [[VAR:%.+]] = load i64
  // CHECK:      %set_bits = and i64 [[VAR]], 127
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
