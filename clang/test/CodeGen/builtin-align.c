// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --function-signature
/// Check the code generation for the alignment builtins
/// To make the test case easier to read, run SROA after generating IR to remove the alloca instructions.
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_VOID_PTR \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-VOID_PTR \
// RUN:   -enable-var-scope '-D$PTRTYPE=i8'
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_FLOAT_PTR \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-FLOAT_PTR \
// RUN:   -enable-var-scope '-D$PTRTYPE=f32'
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_LONG \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-LONG -enable-var-scope
/// Check that we can handle the case where the alignment parameter is wider
/// than the source type (generate a trunc on alignment instead of zext)
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -DTEST_USHORT \
// RUN:   -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-USHORT -enable-var-scope

/// Test for CHERI
// RUN: %cheri_cc1 -DTEST_CAP -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-CAP_POINTER
// RUN: %cheri_cc1 -DTEST_INTCAP -o - -emit-llvm %s -disable-O0-optnone | opt -S -passes=sroa | \
// RUN:   FileCheck %s -check-prefixes CHECK,CHECK-INTCAP

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

#if !__has_feature(cheri)
#define __capability
#endif

// UTC_ARGS: --disable
/// Check that constant initializers work and are correct
_Bool aligned_true = __builtin_is_aligned(1024, 512);
// CHECK: @aligned_true ={{.*}} global i8 1, align 1
_Bool aligned_false = __builtin_is_aligned(123, 512);
// CHECK: @aligned_false ={{.*}} global i8 0, align 1

int down_1 = __builtin_align_down(1023, 32);
// CHECK: @down_1 ={{.*}} global i32 992, align 4
int down_2 = __builtin_align_down(256, 32);
// CHECK: @down_2 ={{.*}} global i32 256, align 4

int up_1 = __builtin_align_up(1023, 32);
// CHECK: @up_1 ={{.*}} global i32 1024, align 4
int up_2 = __builtin_align_up(256, 32);
// CHECK: @up_2 ={{.*}} global i32 256, align 4
// UTC_ARGS: --enable

/// Capture the IR type here to use in the remaining FileCheck captures:
// CHECK-VOID_PTR-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-VOID_PTR-NEXT:  entry:
// CHECK-VOID_PTR-NEXT:    ret ptr null
//
// CHECK-FLOAT_PTR-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-FLOAT_PTR-NEXT:  entry:
// CHECK-FLOAT_PTR-NEXT:    ret ptr null
//
// CHECK-LONG-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-LONG-NEXT:  entry:
// CHECK-LONG-NEXT:    ret i64 0
//
// CHECK-USHORT-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-USHORT-NEXT:  entry:
// CHECK-USHORT-NEXT:    ret i16 0
//
// CHECK-CAP_POINTER-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-CAP_POINTER-NEXT:  entry:
// CHECK-CAP_POINTER-NEXT:    ret ptr addrspace(200) null
//
// CHECK-INTCAP-LABEL: define {{[^@]+}}@get_type() #0
// CHECK-INTCAP-NEXT:  entry:
// CHECK-INTCAP-NEXT:    ret ptr addrspace(200) null
//
TYPE get_type(void) {
  return (TYPE)0;
}

// CHECK-VOID_PTR-LABEL: define {{[^@]+}}@is_aligned
// CHECK-VOID_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-VOID_PTR-NEXT:  entry:
// CHECK-VOID_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-VOID_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-VOID_PTR-NEXT:    [[SRC_ADDR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-VOID_PTR-NEXT:    [[SET_BITS:%.*]] = and i64 [[SRC_ADDR]], [[MASK]]
// CHECK-VOID_PTR-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i64 [[SET_BITS]], 0
// CHECK-VOID_PTR-NEXT:    ret i1 [[IS_ALIGNED]]
//
// CHECK-FLOAT_PTR-LABEL: define {{[^@]+}}@is_aligned
// CHECK-FLOAT_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-FLOAT_PTR-NEXT:  entry:
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-FLOAT_PTR-NEXT:    [[SRC_ADDR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[SET_BITS:%.*]] = and i64 [[SRC_ADDR]], [[MASK]]
// CHECK-FLOAT_PTR-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i64 [[SET_BITS]], 0
// CHECK-FLOAT_PTR-NEXT:    ret i1 [[IS_ALIGNED]]
//
// CHECK-LONG-LABEL: define {{[^@]+}}@is_aligned
// CHECK-LONG-SAME: (i64 noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-LONG-NEXT:  entry:
// CHECK-LONG-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-LONG-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-LONG-NEXT:    [[SET_BITS:%.*]] = and i64 [[PTR]], [[MASK]]
// CHECK-LONG-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i64 [[SET_BITS]], 0
// CHECK-LONG-NEXT:    ret i1 [[IS_ALIGNED]]
//
// CHECK-USHORT-LABEL: define {{[^@]+}}@is_aligned
// CHECK-USHORT-SAME: (i16 noundef zeroext [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-USHORT-NEXT:  entry:
// CHECK-USHORT-NEXT:    [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to i16
// CHECK-USHORT-NEXT:    [[MASK:%.*]] = sub i16 [[ALIGNMENT]], 1
// CHECK-USHORT-NEXT:    [[SET_BITS:%.*]] = and i16 [[PTR]], [[MASK]]
// CHECK-USHORT-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i16 [[SET_BITS]], 0
// CHECK-USHORT-NEXT:    ret i1 [[IS_ALIGNED]]
//
// CHECK-CAP_POINTER-LABEL: define {{[^@]+}}@is_aligned
// CHECK-CAP_POINTER-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-CAP_POINTER-NEXT:  entry:
// CHECK-CAP_POINTER-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-CAP_POINTER-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-CAP_POINTER-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-CAP_POINTER-NEXT:    [[SET_BITS:%.*]] = and i64 [[PTRADDR]], [[MASK]]
// CHECK-CAP_POINTER-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i64 [[SET_BITS]], 0
// CHECK-CAP_POINTER-NEXT:    ret i1 [[IS_ALIGNED]]
//
// CHECK-INTCAP-LABEL: define {{[^@]+}}@is_aligned
// CHECK-INTCAP-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-INTCAP-NEXT:  entry:
// CHECK-INTCAP-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-INTCAP-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-INTCAP-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-INTCAP-NEXT:    [[SET_BITS:%.*]] = and i64 [[PTRADDR]], [[MASK]]
// CHECK-INTCAP-NEXT:    [[IS_ALIGNED:%.*]] = icmp eq i64 [[SET_BITS]], 0
// CHECK-INTCAP-NEXT:    ret i1 [[IS_ALIGNED]]
//
_Bool is_aligned(TYPE ptr, unsigned align) {
  return __builtin_is_aligned(ptr, align);
}

// NOTYET-POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = call [[$TYPE]] @llvm.ptrmask.p0[[$PTRTYPE]].p0i8.i64(ptr [[OVER_BOUNDARY]], [[ALIGN_TYPE]] [[INVERTED_MASK]])
// CHECK-VOID_PTR-LABEL: define {{[^@]+}}@align_up
// CHECK-VOID_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-VOID_PTR-NEXT:  entry:
// CHECK-VOID_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-VOID_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-VOID_PTR-NEXT:    [[INTPTR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-VOID_PTR-NEXT:    [[OVER_BOUNDARY:%.*]] = add i64 [[INTPTR]], [[MASK]]
// CHECK-VOID_PTR-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-VOID_PTR-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-VOID_PTR-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[INTPTR]]
// CHECK-VOID_PTR-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr [[PTR]], i64 [[DIFF]]
// CHECK-VOID_PTR-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-VOID_PTR-NEXT:    ret ptr [[ALIGNED_RESULT]]
//
// CHECK-FLOAT_PTR-LABEL: define {{[^@]+}}@align_up
// CHECK-FLOAT_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-FLOAT_PTR-NEXT:  entry:
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-FLOAT_PTR-NEXT:    [[INTPTR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[OVER_BOUNDARY:%.*]] = add i64 [[INTPTR]], [[MASK]]
// CHECK-FLOAT_PTR-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-FLOAT_PTR-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[INTPTR]]
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr [[PTR]], i64 [[DIFF]]
// CHECK-FLOAT_PTR-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-FLOAT_PTR-NEXT:    ret ptr [[ALIGNED_RESULT]]
//
// CHECK-LONG-LABEL: define {{[^@]+}}@align_up
// CHECK-LONG-SAME: (i64 noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-LONG-NEXT:  entry:
// CHECK-LONG-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-LONG-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-LONG-NEXT:    [[OVER_BOUNDARY:%.*]] = add i64 [[PTR]], [[MASK]]
// CHECK-LONG-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-LONG-NEXT:    [[ALIGNED_RESULT:%.*]] = and i64 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-LONG-NEXT:    ret i64 [[ALIGNED_RESULT]]
//
// CHECK-USHORT-LABEL: define {{[^@]+}}@align_up
// CHECK-USHORT-SAME: (i16 noundef zeroext [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-USHORT-NEXT:  entry:
// CHECK-USHORT-NEXT:    [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to i16
// CHECK-USHORT-NEXT:    [[MASK:%.*]] = sub i16 [[ALIGNMENT]], 1
// CHECK-USHORT-NEXT:    [[OVER_BOUNDARY:%.*]] = add i16 [[PTR]], [[MASK]]
// CHECK-USHORT-NEXT:    [[INVERTED_MASK:%.*]] = xor i16 [[MASK]], -1
// CHECK-USHORT-NEXT:    [[ALIGNED_RESULT:%.*]] = and i16 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-USHORT-NEXT:    ret i16 [[ALIGNED_RESULT]]
//
// CHECK-CAP_POINTER-LABEL: define {{[^@]+}}@align_up
// CHECK-CAP_POINTER-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-CAP_POINTER-NEXT:  entry:
// CHECK-CAP_POINTER-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-CAP_POINTER-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-CAP_POINTER-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-CAP_POINTER-NEXT:    [[OVER_BOUNDARY:%.*]] = add i64 [[PTRADDR]], [[MASK]]
// CHECK-CAP_POINTER-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-CAP_POINTER-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-CAP_POINTER-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[PTRADDR]]
// CHECK-CAP_POINTER-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr addrspace(200) [[PTR]], i64 [[DIFF]]
// CHECK-CAP_POINTER-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr addrspace(200) [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-CAP_POINTER-NEXT:    ret ptr addrspace(200) [[ALIGNED_RESULT]]
//
// CHECK-INTCAP-LABEL: define {{[^@]+}}@align_up
// CHECK-INTCAP-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-INTCAP-NEXT:  entry:
// CHECK-INTCAP-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-INTCAP-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-INTCAP-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-INTCAP-NEXT:    [[OVER_BOUNDARY:%.*]] = add i64 [[PTRADDR]], [[MASK]]
// CHECK-INTCAP-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-INTCAP-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[OVER_BOUNDARY]], [[INVERTED_MASK]]
// CHECK-INTCAP-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[PTRADDR]]
// CHECK-INTCAP-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr i8, ptr addrspace(200) [[PTR]], i64 [[DIFF]]
// CHECK-INTCAP-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr addrspace(200) [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-INTCAP-NEXT:    ret ptr addrspace(200) [[ALIGNED_RESULT]]
//
TYPE align_up(TYPE ptr, unsigned align) {
  return __builtin_align_up(ptr, align);
}

// NOTYET-POINTER-NEXT:  [[ALIGNED_RESULT:%.*]] = call [[$TYPE]] @llvm.ptrmask.p0[[$PTRTYPE]].p0[[$PTRTYPE]].i64([[$TYPE]] [[PTR]], [[ALIGN_TYPE]] [[INVERTED_MASK]])
// CHECK-VOID_PTR-LABEL: define {{[^@]+}}@align_down
// CHECK-VOID_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-VOID_PTR-NEXT:  entry:
// CHECK-VOID_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-VOID_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-VOID_PTR-NEXT:    [[INTPTR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-VOID_PTR-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-VOID_PTR-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[INTPTR]], [[INVERTED_MASK]]
// CHECK-VOID_PTR-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[INTPTR]]
// CHECK-VOID_PTR-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr [[PTR]], i64 [[DIFF]]
// CHECK-VOID_PTR-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-VOID_PTR-NEXT:    ret ptr [[ALIGNED_RESULT]]
//
// CHECK-FLOAT_PTR-LABEL: define {{[^@]+}}@align_down
// CHECK-FLOAT_PTR-SAME: (ptr noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-FLOAT_PTR-NEXT:  entry:
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-FLOAT_PTR-NEXT:    [[INTPTR:%.*]] = ptrtoint ptr [[PTR]] to i64
// CHECK-FLOAT_PTR-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[INTPTR]], [[INVERTED_MASK]]
// CHECK-FLOAT_PTR-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[INTPTR]]
// CHECK-FLOAT_PTR-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr [[PTR]], i64 [[DIFF]]
// CHECK-FLOAT_PTR-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-FLOAT_PTR-NEXT:    ret ptr [[ALIGNED_RESULT]]
//
// CHECK-LONG-LABEL: define {{[^@]+}}@align_down
// CHECK-LONG-SAME: (i64 noundef [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-LONG-NEXT:  entry:
// CHECK-LONG-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-LONG-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-LONG-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-LONG-NEXT:    [[ALIGNED_RESULT:%.*]] = and i64 [[PTR]], [[INVERTED_MASK]]
// CHECK-LONG-NEXT:    ret i64 [[ALIGNED_RESULT]]
//
// CHECK-USHORT-LABEL: define {{[^@]+}}@align_down
// CHECK-USHORT-SAME: (i16 noundef zeroext [[PTR:%.*]], i32 noundef [[ALIGN:%.*]]) #0
// CHECK-USHORT-NEXT:  entry:
// CHECK-USHORT-NEXT:    [[ALIGNMENT:%.*]] = trunc i32 [[ALIGN]] to i16
// CHECK-USHORT-NEXT:    [[MASK:%.*]] = sub i16 [[ALIGNMENT]], 1
// CHECK-USHORT-NEXT:    [[INVERTED_MASK:%.*]] = xor i16 [[MASK]], -1
// CHECK-USHORT-NEXT:    [[ALIGNED_RESULT:%.*]] = and i16 [[PTR]], [[INVERTED_MASK]]
// CHECK-USHORT-NEXT:    ret i16 [[ALIGNED_RESULT]]
//
// CHECK-CAP_POINTER-LABEL: define {{[^@]+}}@align_down
// CHECK-CAP_POINTER-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-CAP_POINTER-NEXT:  entry:
// CHECK-CAP_POINTER-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-CAP_POINTER-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-CAP_POINTER-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-CAP_POINTER-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-CAP_POINTER-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[PTRADDR]], [[INVERTED_MASK]]
// CHECK-CAP_POINTER-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[PTRADDR]]
// CHECK-CAP_POINTER-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr inbounds i8, ptr addrspace(200) [[PTR]], i64 [[DIFF]]
// CHECK-CAP_POINTER-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr addrspace(200) [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-CAP_POINTER-NEXT:    ret ptr addrspace(200) [[ALIGNED_RESULT]]
//
// CHECK-INTCAP-LABEL: define {{[^@]+}}@align_down
// CHECK-INTCAP-SAME: (ptr addrspace(200) noundef [[PTR:%.*]], i32 noundef signext [[ALIGN:%.*]]) #0
// CHECK-INTCAP-NEXT:  entry:
// CHECK-INTCAP-NEXT:    [[ALIGNMENT:%.*]] = zext i32 [[ALIGN]] to i64
// CHECK-INTCAP-NEXT:    [[MASK:%.*]] = sub i64 [[ALIGNMENT]], 1
// CHECK-INTCAP-NEXT:    [[PTRADDR:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[PTR]])
// CHECK-INTCAP-NEXT:    [[INVERTED_MASK:%.*]] = xor i64 [[MASK]], -1
// CHECK-INTCAP-NEXT:    [[ALIGNED_INTPTR:%.*]] = and i64 [[PTRADDR]], [[INVERTED_MASK]]
// CHECK-INTCAP-NEXT:    [[DIFF:%.*]] = sub i64 [[ALIGNED_INTPTR]], [[PTRADDR]]
// CHECK-INTCAP-NEXT:    [[ALIGNED_RESULT:%.*]] = getelementptr i8, ptr addrspace(200) [[PTR]], i64 [[DIFF]]
// CHECK-INTCAP-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr addrspace(200) [[ALIGNED_RESULT]], i64 [[ALIGNMENT]]) ]
// CHECK-INTCAP-NEXT:    ret ptr addrspace(200) [[ALIGNED_RESULT]]
//
TYPE align_down(TYPE ptr, unsigned align) {
  return __builtin_align_down(ptr, align);
}
