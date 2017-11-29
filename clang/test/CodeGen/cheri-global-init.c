// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm  -o - -no-cheri-linker | %cheri_FileCheck --check-prefix=CHECK-NO-LINKER %s
// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm  -o - -cheri-linker | %cheri_FileCheck --check-prefix=CHECK-LINKER %s

int a[5];
int *b[] = {&a[2], &a[1], a};
// CHECK-NO-LINKER: __cxx_global_var_init
// CHECK-LINKER: @b = addrspace(200) global [3 x i32 addrspace(200)*] [i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 8) to i32 addrspace(200)*), i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 4) to i32 addrspace(200)*), i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)], align [[$CAP_SIZE]]

