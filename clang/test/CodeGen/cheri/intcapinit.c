// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm | %cheri_FileCheck %s

// CHECK: @x = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), align [[#CAP_SIZE]]
__intcap_t x = 1;
// CHECK: @y = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), align [[#CAP_SIZE]]
void *y = (void*)1;


