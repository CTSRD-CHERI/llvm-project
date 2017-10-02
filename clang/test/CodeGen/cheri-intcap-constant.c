// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck -check-prefix PURECAP %s



int i;

const void* foo = &i;
const void* foo1 = (void*)&i;
// Casting via uintptr_t should not be an error as this is a common pattern (at least in C) to remove const/volatile
const void* foo2 = (void*)(__intcap_t)&i;
const void* foo3 = (void*)(__intcap_t)(const void*)&i;

#ifndef __CHERI_PURE_CAPABILITY__
const void* foo4 = (void*)(long)&i;
#endif

// CHECK: @i = common global i32 0, align 4
// CHECK: @foo = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo1 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo2 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo3 = global i8* bitcast (i32* @i to i8*), align 8
// CHECK: @foo4 = global i8* bitcast (i32* @i to i8*), align 8

// PURECAP: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo, align [[$CAP_SIZE:16|32]]
// PURECAP: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo1, align [[$CAP_SIZE:16|32]]
// PURECAP: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo2, align [[$CAP_SIZE:16|32]]
// PURECAP: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @i to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* @foo3, align [[$CAP_SIZE:16|32]]
