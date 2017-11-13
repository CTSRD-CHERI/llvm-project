// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O0 | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O2 | FileCheck %s -check-prefix CHECK-OPT
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O3 | FileCheck %s -check-prefix CHECK-OPT


#define A (__intcap_t)1
#define B (__intcap_t)(void*)2
#define C (__intcap_t)3
#define D (__intcap_t)(void*)5

int x(__intcap_t y)
{
  // CHECK-LABEL: define i32 @x(i8 addrspace(200)*
  // CHECK: [[SWITCH_VAR:%.+]] = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* {{%.+}})
  // CHECK: switch i64 [[SWITCH_VAR]], label {{%.+}} [
  switch (y)
  {
    // CHECK-NEXT: i64 1, label %[[BB0:.+]]
    case A: return 4;
    // CHECK-NEXT: i64 2, label %[[BB1:.+]]
    case B: return 5;
    // CHECK-NEXT: i64 3, label %[[BB2:.+]]
    // CHECK-NEXT: i64 4, label %[[BB2]]
    // CHECK-NEXT: i64 5, label %[[BB2]]
    case C...D: return 6;
  }
  // CHECK: [[BB0]]:
  // CHECK-NEXT: store i32 4
  // CHECK: [[BB1]]:
  // CHECK-NEXT: store i32 5
  // CHECK: [[BB2]]:
  // CHECK-NEXT: store i32 6
  return 0;
}

int y(void) {
  // CHECK-LABEL: define i32 @y()
  __intcap_t foo = C;
  // CHECK: [[SWITCH_VAR:%.+]] = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* {{%.+}})
  // CHECK: switch i64 [[SWITCH_VAR]], label {{%.+}} [
  switch (foo)
  {
    // CHECK: i64 1, label %[[BB0:.+]]
    case A: return 4;
    // CHECK-NEXT: i64 2, label %[[BB1:.+]]
    case B: return 5;
    // CHECK-NEXT: i64 3, label %[[BB2:.+]]
    // CHECK-NEXT: i64 4, label %[[BB2]]
    // CHECK-NEXT: i64 5, label %[[BB2]]
    case C...D: return 6;
  }
  // CHECK: [[BB0]]:
  // CHECK-NEXT: store i32 4
  // CHECK: [[BB1]]:
  // CHECK-NEXT: store i32 5
  // CHECK: [[BB2]]:
  // CHECK-NEXT: store i32 6

  // XXXAR: this is currently not optimized after the fix to https://github.com/CTSRD-CHERI/clang/issues/132
  // with optimization this switch gets constant folded:
  // DONTCHECK-OPT-LABEL: define i32 @y()
  // DONTCHECK-OPT:          ret i32 6
  return 0;
}

char buf[16];

int z_long(void) {
  // CHECK-OPT-LABEL: define i32 @z_long()
  // CHECK-OPT:      [[SWITCHVAL:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* getelementptr inbounds ([16 x i8], [16 x i8] addrspace(200)* @buf, i64 0, i64 0))
  // CHECK-OPT-NEXT: switch i64 [[SWITCHVAL]], label {{%.+}} [

  const char* switchval = buf;
  switch ((long)switchval) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    default: return -1;
  }
}

int z_intcap(void) {
  // Check that we actually switch on the virtual address and not the offset:
  // CHECK-OPT-LABEL: define i32 @z_intcap()
  // CHECK-OPT-NOT: ret i32 4
  // CHECK-OPT:      [[SWITCHVAL:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* getelementptr inbounds ([16 x i8], [16 x i8] addrspace(200)* @buf, i64 0, i64 0))
  // CHECK-OPT-NEXT: switch i64 [[SWITCHVAL]], label {{%.+}} [
  const char* switchval = buf;
  switch ((__intcap_t)switchval) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    default: return -1;
  }

}

int z_fixed_offset(void) {
  // Offset is known, so this could be optimized to a base.get and the case statements get adjusted
  // This was the previous behaviour when using separate base.get and offset.get intrinsics, but I'm not sure it makes sense to keep it.
  // CHECK-OPT-LABEL: define i32 @z_fixed_offset()
  // CHECK-OPT-NOT: ret i32 4
  // CHECK-OPT: [[CAP:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* getelementptr inbounds ([16 x i8], [16 x i8] addrspace(200)* @buf, i64 0, i64 0), i64 4)
  // CHECK-OPT-NEXT: [[SWITCHVAL:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP]])
  // CHECK-OPT-NEXT: switch i64 [[SWITCHVAL]], label {{%.+}} [
  // CHECK-OPT-NEXT:   i64 0
  // CHECK-OPT-NEXT:   i64 1
  // CHECK-OPT-NEXT:   i64 2
  // CHECK-OPT-NEXT:   i64 3
  // CHECK-OPT-NEXT:   i64 4
  // CHECK-OPT-NEXT: ]

  const char* switchval = __builtin_cheri_offset_set(buf, 4);
  switch ((__intcap_t)switchval) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    default: return -1;
  }
}
