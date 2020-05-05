/// Check the code generation for the alignment builtins
/// To make the test case easier to read, run SROA after generating IR to remove the alloca instructions.
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_VOID_PTR \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,POINTER,NONCAP_PTR,ALIGNMENT_EXT \
// RUN:   -enable-var-scope '-D$PTRTYPE=i8'
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_FLOAT_PTR \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,POINTER,NONCAP_PTR,NON_I8_POINTER,ALIGNMENT_EXT \
// RUN:   -enable-var-scope '-D$PTRTYPE=f32'
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_LONG \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,INTEGER,ALIGNMENT_EXT -enable-var-scope
/// Check that we can handle the case where the alignment parameter is wider
/// than the source type (generate a trunc on alignment instead of zext)
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_USHORT \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,INTEGER,ALIGNMENT_TRUNC -enable-var-scope

/// Test for CHERI
// RUN: %cheri_cc1 -DTEST_CAP -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,POINTER,CAP_POINTER,CAPABILITY,ALIGNMENT_EXT
// RUN: %cheri_cc1 -DTEST_INTCAP -o - -emit-llvm %s -disable-O0-optnone | opt -S -sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,POINTER,INTCAP,CAPABILITY,ALIGNMENT_EXT

#ifdef TEST_VOID_PTR
#define TYPE void *
#elif defined(TEST_FLOAT_PTR)
#define TYPE float *
#elif defined(TEST_LONG)
#define TYPE long
#elif defined(TEST_CAP)
#define TYPE void *__capability
#elif defined(TEST_INTCAP)
#define TYPE __intcap_t
#elif defined(TEST_USHORT)
#define TYPE unsigned short
#else
#error MISSING TYPE
#endif

#ifndef __CHERI__
#define __capability
#endif

/// Check that constant initializers work and are correct
_Bool aligned_true = __builtin_is_aligned(1024, 512);
// CHECK: @aligned_true = global i8 1, align 1
_Bool aligned_false = __builtin_is_aligned(123, 512);
// CHECK: @aligned_false = global i8 0, align 1

int down_1 = __builtin_align_down(1023, 32);
// CHECK: @down_1 = global i32 992, align 4
int down_2 = __builtin_align_down(256, 32);
// CHECK: @down_2 = global i32 256, align 4

int up_1 = __builtin_align_up(1023, 32);
// CHECK: @up_1 = global i32 1024, align 4
int up_2 = __builtin_align_up(256, 32);
// CHECK: @up_2 = global i32 256, align 4

/// Capture the IR type here to use in the remaining FileCheck captures:
// CHECK: define {{[^@]+}}@get_type() #0
// CHECK-NEXT:  entry:
// POINTER-NEXT:   ret [[$TYPE:.+]] null
// INTEGER-NEXT:   ret [[$TYPE:.+]] 0
//
TYPE get_type(void) {
  return (TYPE)0;
}

// CHECK: define {{[^@]+}}@get_i8ptr_type() #0
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret [[$I8PTR_TYPE:.+]] null
//
void* __capability get_i8ptr_type(void) {
  return 0;
}

// CHECK-LABEL: define {{[^@]+}}@is_aligned
// CHECK-SAME: ([[$TYPE]] {{[^%]*}}[[PTR:%.*]], i32 {{[^%]*}}[[ALIGN:%.*]]) #0
// CHECK-NEXT:  entry:
// ALIGNMENT_EXT-NEXT:   [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to [[ALIGN_TYPE:i64]]
// ALIGNMENT_TRUNC-NEXT: [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to [[ALIGN_TYPE:i16]]
// CHECK-NEXT:           [[MASK:%.*]] = sub [[ALIGN_TYPE]] [[ALIGNMENT]], 1
// NONCAP_PTR-NEXT:      [[PTR:%.*]] = ptrtoint [[$TYPE]] %ptr to [[ALIGN_TYPE]]
// CAPABILITY-NEXT:      [[PTR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64([[$TYPE]] %ptr)
// CHECK-NEXT:           [[SET_BITS:%.*]] = and [[ALIGN_TYPE]] [[PTR]], [[MASK]]
// CHECK-NEXT:           [[IS_ALIGNED:%.*]] = icmp eq [[ALIGN_TYPE]] [[SET_BITS]], 0
// CHECK-NEXT:           ret i1 [[IS_ALIGNED]]
//
_Bool is_aligned(TYPE ptr, unsigned align) {
  return __builtin_is_aligned(ptr, align);
}

// CHECK-LABEL: define {{[^@]+}}@align_up
// CHECK-SAME: ([[$TYPE]] {{[^%]*}}[[PTR:%.*]], i32 {{[^%]*}}[[ALIGN:%.*]]) #0
// CHECK-NEXT:  entry:
// ALIGNMENT_EXT-NEXT:   [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to [[ALIGN_TYPE:i64]]
// ALIGNMENT_TRUNC-NEXT: [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to [[ALIGN_TYPE:i16]]
// CHECK-NEXT:           [[MASK:%.*]] = sub [[ALIGN_TYPE]] [[ALIGNMENT]], 1
// INTEGER-NEXT:         [[OVER_BOUNDARY:%.*]] = add [[$TYPE]] [[PTR]], [[MASK]]
// NOTYET-POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = call [[$TYPE]] @llvm.ptrmask.p0[[$PTRTYPE]].p0i8.i64([[$I8PTR_TYPE]] [[OVER_BOUNDARY]], [[ALIGN_TYPE]] [[INVERTED_MASK]])
// NONCAP_PTR-NEXT:      [[INTPTR:%.*]] = ptrtoint [[$TYPE]] [[PTR]] to [[ALIGN_TYPE]]
// CAPABILITY-NEXT:      [[INTPTR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64([[$TYPE]] [[PTR]])
// POINTER-NEXT:         [[OVER_BOUNDARY:%.*]] = add [[ALIGN_TYPE]] [[INTPTR]], [[MASK]]
// CHECK-NEXT:           [[INVERTED_MASK:%.*]] = xor [[ALIGN_TYPE]] [[MASK]], -1
// CHECK-NEXT:           [[ALIGNED_RESULT:%.*]] = and [[ALIGN_TYPE]] [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// POINTER-NEXT:         [[DIFF:%.*]] = sub i64 [[ALIGNED_RESULT]], [[INTPTR]]
// NON_I8_POINTER-NEXT:  [[PTR:%.*]] = bitcast [[$TYPE]] {{%.*}} to [[$I8PTR_TYPE]]
/// Note: we can't set the inbounds flag for incap_t
// INTCAP-NEXT:          [[ALIGNED_RESULT:%.*]] = getelementptr i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// CAP_POINTER-NEXT:     [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// NONCAP_PTR-NEXT:      [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// NON_I8_POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = bitcast [[$I8PTR_TYPE]] {{%.*}} to [[$TYPE]]
// POINTER-NEXT:         [[ASSUME_MASK:%.*]] = sub i64 %alignment, 1
// POINTER-NEXT:         [[ASSUME_INTPTR:%.*]]= ptrtoint [[$TYPE]] [[ALIGNED_RESULT]] to i64
// POINTER-NEXT:         [[MASKEDPTR:%.*]] = and i64 %ptrint, [[ASSUME_MASK]]
// POINTER-NEXT:         [[MASKEDCOND:%.*]] = icmp eq i64 [[MASKEDPTR]], 0
// POINTER-NEXT:         call void @llvm.assume(i1 [[MASKEDCOND]])
// CHECK-NEXT:           ret [[$TYPE]] [[ALIGNED_RESULT]]
//
TYPE align_up(TYPE ptr, unsigned align) {
  return __builtin_align_up(ptr, align);
}

// CHECK-LABEL: define {{[^@]+}}@align_down
// CHECK-SAME: ([[$TYPE]] {{[^%]*}}[[PTR:%.*]], i32 {{[^%]*}}[[ALIGN:%.*]]) #0
// CHECK-NEXT:  entry:
// ALIGNMENT_EXT-NEXT:   [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to [[ALIGN_TYPE:i64]]
// ALIGNMENT_TRUNC-NEXT: [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to [[ALIGN_TYPE:i16]]
// CHECK-NEXT:           [[MASK:%.*]] = sub [[ALIGN_TYPE]] [[ALIGNMENT]], 1
// NOTYET-POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = call [[$TYPE]] @llvm.ptrmask.p0[[$PTRTYPE]].p0[[$PTRTYPE]].i64([[$TYPE]] [[PTR]], [[ALIGN_TYPE]] [[INVERTED_MASK]])
// NONCAP_PTR-NEXT:      [[INTPTR:%.*]] = ptrtoint [[$TYPE]] [[PTR]] to [[ALIGN_TYPE]]
// CAPABILITY-NEXT:      [[INTPTR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64([[$TYPE]] [[PTR]])
// CHECK-NEXT:           [[INVERTED_MASK:%.*]] = xor [[ALIGN_TYPE]] [[MASK]], -1
// POINTER-NEXT:         [[ALIGNED_INTPTR:%.*]] = and [[ALIGN_TYPE]] [[INTPTR]], [[INVERTED_MASK]]
// POINTER-NEXT:         [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[INTPTR]]
// NON_I8_POINTER-NEXT:  [[PTR:%.*]] = bitcast [[$TYPE]] {{%.*}} to [[$I8PTR_TYPE]]
/// Note: we can't set the inbounds flag for incap_t
// INTCAP-NEXT:          [[ALIGNED_RESULT:%.*]] = getelementptr i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// CAP_POINTER-NEXT:     [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// NONCAP_PTR-NEXT:      [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, [[$I8PTR_TYPE]] [[PTR]], i64 [[DIFF]]
// NON_I8_POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = bitcast [[$I8PTR_TYPE]] {{%.*}} to [[$TYPE]]
// INTEGER-NEXT:         [[ALIGNED_RESULT:%.*]] = and [[ALIGN_TYPE]] [[PTR]], [[INVERTED_MASK]]
// POINTER-NEXT:         [[ASSUME_MASK:%.*]] = sub i64 %alignment, 1
// POINTER-NEXT:         [[ASSUME_INTPTR:%.*]]= ptrtoint [[$TYPE]] [[ALIGNED_RESULT]] to i64
// POINTER-NEXT:         [[MASKEDPTR:%.*]] = and i64 %ptrint, [[ASSUME_MASK]]
// POINTER-NEXT:         [[MASKEDCOND:%.*]] = icmp eq i64 [[MASKEDPTR]], 0
// POINTER-NEXT:         call void @llvm.assume(i1 [[MASKEDCOND]])
// CHECK-NEXT:           ret [[$TYPE]] [[ALIGNED_RESULT]]
//
TYPE align_down(TYPE ptr, unsigned align) {
  return __builtin_align_down(ptr, align);
}
