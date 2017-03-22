// RUN: %clang %s -mabi=purecap -cheri-linker -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s

// CHECK: @x = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align 32
__intcap_t x = 1;
// CHECK: @y = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align 32
void *y = (void*)1;


