// RUN: %cheri_cc1 -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes SLOW,BOTH
// RUN: %cheri_cc1 -o - -O2 -emit-llvm %s | FileCheck %s -check-prefixes OPT,BOTH

_Bool is_aligned(void *x) {
  // BOTH-LABEL: is_aligned(
  // BOTH:      [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // BOTH-NEXT: %set_bits = and i64 [[VAR]], 31
  // BOTH-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // BOTH-NEXT: ret i1 %is_aligned
  return __builtin_is_aligned(x, 32);
}

_Bool is_aligned2(void *x) {
  // BOTH-LABEL: is_aligned2(
  // BOTH:     [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // BOTH-NEXT: %set_bits = and i64 [[VAR]], 127
  // BOTH-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // BOTH-NEXT: ret i1 %is_aligned
  return __builtin_is_p2aligned(x, 7);
}

_Bool is_aligned3(void *x, int p2align) {
  // BOTH-LABEL: is_aligned3(
  // BOTH:     [[VAR:%.+]] = ptrtoint i8* %{{.+}} to i64
  // SLOW-NEXT: %2 = load i32, i32* %p2align.addr, align 4
  // SLOW-NEXT: %pow2 = zext i32 %2 to i64
  // OPT-NEXT:  %pow2 = zext i32 %p2align to i64
  // BOTH-NEXT: %alignment = shl i64 1, %pow2
  // SLOW-NEXT: %mask = sub i64 %alignment, 1
  // OPT-NEXT: %mask = add i64 %alignment, -1
  // SLOW-NEXT: %set_bits = and i64 [[VAR]], %mask
  // OPT-NEXT: %set_bits = and i64 %mask, [[VAR]]
  // BOTH-NEXT: %is_aligned = icmp eq i64 %set_bits, 0
  // BOTH-NEXT: ret i1 %is_aligned
  return __builtin_is_p2aligned(x, p2align);
}

// Check that constants get folded
_Bool opt_is_aligned1(void *x) {
  // BOTH-LABEL: opt_is_aligned1(
  // SLOW:   [[PHI:%.+]] = phi i1 [ false, %{{.+}} ], [ %{{.+}}, %{{.+}} ]
  // SLOW: ret i1 [[PHI]]
  // OPT: ret i1 true
  return __builtin_is_p2aligned(x, 0) && __builtin_is_aligned(x, 1);
}

_Bool opt_is_aligned2() {
  // This expression gets folded even at -O0:
  // BOTH-LABEL: opt_is_aligned2(
  // BOTH: ret i1 false
  return __builtin_is_p2aligned(7, 3);
}

_Bool opt_is_aligned3() {
  // This expression gets folded even at -O0:
  // BOTH-LABEL: opt_is_aligned3(
  // BOTH: ret i1 true
  return __builtin_is_aligned(1024, 512);
}
