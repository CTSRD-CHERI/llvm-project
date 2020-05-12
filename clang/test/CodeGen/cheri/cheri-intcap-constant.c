// RUN: %cheri_cc1 -emit-llvm -o - %s -verify | %cheri_FileCheck %s -check-prefixes CHECK,HYBRID
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s -verify | %cheri_FileCheck -check-prefixes CHECK,PURECAP %s

int i;

const void* foo = &i;
const void* foo1 = (void*)&i;
// Casting via uintptr_t should not be an error as this is a common pattern (at least in C) to remove const/volatile
const void* foo2 = (void*)(__intcap_t)&i;
const void* foo3 = (void*)(__intcap_t)(const void*)&i;

#ifndef __CHERI_PURE_CAPABILITY__
const void* foo4 = (void*)(long)&i;
#endif

// Check that constant initialization works even for binary operations
const __uintcap_t thinFlag = 1;
const __uintcap_t reservedFlag = 2;
const __uintcap_t flags = thinFlag | reservedFlag; // expected-warning{{binary expression on capability types}}

// HYBRID: @i = global i32 0, align 4
// HYBRID-NEXT: @foo = global i8* bitcast (i32* @i to i8*), align 8
// HYBRID-NEXT: @foo1 = global i8* bitcast (i32* @i to i8*), align 8
// HYBRID-NEXT: @foo2 = global i8* bitcast (i32* @i to i8*), align 8
// HYBRID-NEXT: @foo3 = global i8* bitcast (i32* @i to i8*), align 8
// HYBRID-NEXT: @foo4 = global i8* bitcast (i32* @i to i8*), align 8
// HYBRID-NEXT: @thinFlag = constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), align [[#CAP_SIZE]]
// HYBRID-NEXT: @reservedFlag = constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2), align [[#CAP_SIZE]]
// HYBRID-NEXT: @flags = constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 3), align [[#CAP_SIZE]]

// PURECAP: @i = addrspace(200) global i32 0, align 4
// PURECAP-NEXT: @foo  = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP-NEXT: @foo1 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP-NEXT: @foo2 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP-NEXT: @foo3 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP-NEXT: @thinFlag = addrspace(200) constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), align [[#CAP_SIZE]]
// PURECAP-NEXT: @reservedFlag = addrspace(200) constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2), align [[#CAP_SIZE]]
// PURECAP-NEXT: @flags = addrspace(200) constant i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 3), align [[#CAP_SIZE]]

// CHECK-EMPTY:
// CHECK-NEXT: Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define void @set_one(
void set_one(__intcap_t *arg) {
  *arg = 1;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1)
  *arg = (__intcap_t)__builtin_cheri_offset_increment(0, 2);
  // CHECK: getelementptr (i8, i8 addrspace(200)* null, i64 2)
}
