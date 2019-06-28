// RUN: %cheri_cc1 -emit-llvm -o - %s | %cheri_FileCheck %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s -no-cheri-linker | %cheri_FileCheck -check-prefix PURECAP-NOLINKER %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | %cheri_FileCheck -check-prefix PURECAP %s



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
const __uintcap_t flags = thinFlag | reservedFlag;

// CHECK: @i = common global i32 0, align 4
// CHECK: @foo = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo1 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo2 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo3 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo4 = global i8* bitcast (i32* @i to i8*), align 8

// CHECK: @thinFlag = constant i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align [[#CAP_SIZE]]
// CHECK: @reservedFlag = constant i8 addrspace(200)* inttoptr (i64 2 to i8 addrspace(200)*), align [[#CAP_SIZE]]
// CHECK: @flags = constant i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*), align [[#CAP_SIZE]]

// PURECAP: @i = common addrspace(200) global i32 0, align 4
// PURECAP: @foo  = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP: @foo1 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP: @foo2 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]
// PURECAP: @foo3 = addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), align [[#CAP_SIZE]]

// PURECAP-NOLINKER: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo, align [[#CAP_SIZE]]
// PURECAP-NOLINKER: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo1, align [[#CAP_SIZE]]
// PURECAP-NOLINKER: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo2, align [[#CAP_SIZE]]
// PURECAP-NOLINKER: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo3, align [[#CAP_SIZE]]
