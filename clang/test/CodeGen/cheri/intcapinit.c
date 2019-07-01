// RUN: %cheri_purecap_cc1 %s -cheri-linker -o - -emit-llvm | %cheri_FileCheck %s

// CHECK: @x = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align [[#CAP_SIZE]]
__intcap_t x = 1;
// CHECK: @y = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align [[#CAP_SIZE]]
void *y = (void*)1;


