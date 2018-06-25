// RUN: %cheri_purecap_cc1 -o - -O2 -emit-llvm  %s | FileCheck %s
// Found while trying to use the builtin in QtBase
char* test1(char* c, int b) {
  // CHECK-LABEL: define i8 addrspace(200)* @_Z5test1U3capPci(
  return __builtin_p2align_down(c, b);
  // CHECK:      [[VAR0:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[C:%.+]])
  // CHECK-NEXT: [[POW2:%.+]] = zext i32 [[B:%.+]] to i64
  // CHECK-NEXT: [[NOTMASK:%.+]] =  shl nsw i64 -1, [[POW2]]
  // CHECK-NEXT: [[MASK:%.+]] = xor i64 [[NOTMASK]], -1
  // CHECK-NEXT: [[UNALIGNED_BITS:%.+]] = and i64 [[VAR0]], [[MASK]]
  // CHECK-NEXT: [[SUB:%.+]] = sub i64 0, [[UNALIGNED_BITS]]
  // CHECK-NEXT: [[ALIGNED_CAP:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[C]], i64 [[SUB]]
  // CHECK-NEXT: ret i8 addrspace(200)* [[ALIGNED_CAP]]
}

// Found while compiling libnv
__uintcap_t test2(__uintcap_t value) {
  // CHECK-LABEL: define i8 addrspace(200)* @_Z5test2u11__uintcap_t(
  // There should be two casts from capability to address (one for size, one for
  // alignment). The alignment one could be an offset get instead but this doesn't
  // really make any difference since Sema validates that it is a valid capability.

  // CHECK:      [[VAR0:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[VALUE:%.+]])
  // CHECK-NEXT: [[UNALIGNED_BITS:%.+]] = and i64 [[VAR0]], 3
  // CHECK-NEXT: [[IS_ALIGNED:%.+]] = icmp eq i64 [[UNALIGNED_BITS]], 0
  // CHECK-NEXT: [[MISSING_BITS:%.+]] = sub nsw i64 4, [[UNALIGNED_BITS]]
  // CHECK-NEXT: [[ALIGNED_CAP:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[VALUE]], i64 [[MISSING_BITS]]
  // CHECK-NEXT: [[RESULT:%.+]] = select i1 [[IS_ALIGNED]], i8 addrspace(200)* [[VALUE]], i8 addrspace(200)* [[ALIGNED_CAP]]
  // CHECK-NEXT: ret i8 addrspace(200)* [[RESULT]]

  return __builtin_align_up(value, (__uintcap_t)sizeof(0));
}
