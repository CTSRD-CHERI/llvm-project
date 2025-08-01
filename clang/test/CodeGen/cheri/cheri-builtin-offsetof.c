// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --version 2
// RUN: %riscv32_cheri_purecap_cc1 -o - -emit-llvm -O0 -disable-O0-optnone %s | \
// RUN:   opt -passes=mem2reg,instcombine -S -o - | FileCheck %s --check-prefixes=RV32
// RUN: %riscv64_cheri_purecap_cc1 -o - -emit-llvm -O0 -disable-O0-optnone %s | \
// RUN:   opt -passes=mem2reg,instcombine -S -o - | FileCheck %s --check-prefixes=RV64

struct foo {
  unsigned int dummy;
  void *children[2];
};

// RV32-LABEL: define dso_local i32 @off_with_intcap
// RV32-SAME: (ptr addrspace(200) noundef [[BIT:%.*]]) addrspace(200) #[[ATTR0:[0-9]+]] {
// RV32-NEXT:  entry:
// RV32-NEXT:    [[TMP0:%.*]] = call i32 @llvm.cheri.cap.address.get.i32(ptr addrspace(200) [[BIT]])
// RV32-NEXT:    [[AND:%.*]] = shl i32 [[TMP0]], 3
// RV32-NEXT:    [[TMP1:%.*]] = and i32 [[AND]], 8
// RV32-NEXT:    [[TMP2:%.*]] = add nuw nsw i32 [[TMP1]], 8
// RV32-NEXT:    ret i32 [[TMP2]]
//
// RV64-LABEL: define dso_local i64 @off_with_intcap
// RV64-SAME: (ptr addrspace(200) noundef [[BIT:%.*]]) addrspace(200) #[[ATTR0:[0-9]+]] {
// RV64-NEXT:  entry:
// RV64-NEXT:    [[TMP0:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[BIT]])
// RV64-NEXT:    [[AND:%.*]] = shl i64 [[TMP0]], 4
// RV64-NEXT:    [[TMP1:%.*]] = and i64 [[AND]], 16
// RV64-NEXT:    [[TMP2:%.*]] = add nuw nsw i64 [[TMP1]], 16
// RV64-NEXT:    ret i64 [[TMP2]]
//
unsigned long off_with_intcap(unsigned __intcap bit) {
  return __builtin_offsetof(struct foo, children[bit & 1]);
}

// RV32-LABEL: define dso_local i32 @off_with_int
// RV32-SAME: (i32 noundef [[BIT:%.*]]) addrspace(200) #[[ATTR0]] {
// RV32-NEXT:  entry:
// RV32-NEXT:    [[AND:%.*]] = shl i32 [[BIT]], 3
// RV32-NEXT:    [[TMP0:%.*]] = and i32 [[AND]], 8
// RV32-NEXT:    [[TMP1:%.*]] = add nuw nsw i32 [[TMP0]], 8
// RV32-NEXT:    ret i32 [[TMP1]]
//
// RV64-LABEL: define dso_local i64 @off_with_int
// RV64-SAME: (i32 noundef signext [[BIT:%.*]]) addrspace(200) #[[ATTR0]] {
// RV64-NEXT:  entry:
// RV64-NEXT:    [[AND:%.*]] = shl i32 [[BIT]], 4
// RV64-NEXT:    [[TMP0:%.*]] = and i32 [[AND]], 16
// RV64-NEXT:    [[NARROW:%.*]] = add nuw nsw i32 [[TMP0]], 16
// RV64-NEXT:    [[TMP1:%.*]] = zext i32 [[NARROW]] to i64
// RV64-NEXT:    ret i64 [[TMP1]]
//
unsigned long off_with_int(unsigned int bit) {
  return __builtin_offsetof(struct foo, children[bit & 1]);
}
