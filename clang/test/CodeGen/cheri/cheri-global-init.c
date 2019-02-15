// RUN: %cheri_cc1 %s "-target-abi" "purecap" -mllvm -cheri-cap-table-abi=legacy -emit-llvm  -o - -no-cheri-linker | %cheri_FileCheck --check-prefixes=CHECK-NO-LINKER,CHECK-NO-LINKER-LEGACY %s
// RUN: %cheri_cc1 %s "-target-abi" "purecap" -mllvm -cheri-cap-table-abi=pcrel -emit-llvm  -o - -no-cheri-linker | %cheri_FileCheck --check-prefixes=CHECK-NO-LINKER,CHECK-NO-LINKER-PCREL %s
// RUN: %cheri_cc1 %s "-target-abi" "purecap" -mllvm -cheri-cap-table-abi=legacy -emit-llvm  -o - -cheri-linker | %cheri_FileCheck --check-prefix=CHECK-LINKER %s
// RUN: %cheri_cc1 %s "-target-abi" "purecap" -mllvm -cheri-cap-table-abi=pcrel -emit-llvm  -o - -cheri-linker | %cheri_FileCheck --check-prefix=CHECK-LINKER %s

int a[5];
int *b[] = {&a[2], &a[1], a};
// CHECK-NO-LINKER: @llvm.global_ctors = appending addrspace(200) global [1 x { i32, void ()*, i8 addrspace(200)* }]
// CHECK-NO-LINKER-SAME: [{ i32, void ()*, i8 addrspace(200)* } { i32 65535,
// CHECK-NO-LINKER-LEGACY-SAME:  void ()* @_GLOBAL__sub_I_cheri_global_init.c,
// CHECK-NO-LINKER-PCREL-SAME:  void ()* addrspacecast (void () addrspace(200)* @_GLOBAL__sub_I_cheri_global_init.c to void ()*),
// CHECK-NO-LINKER-SAME: i8 addrspace(200)* null }]
// CHECK-NO-LINKER: __cxx_global_var_init
// CHECK-LINKER: @b = addrspace(200) global [3 x i32 addrspace(200)*] [i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 8) to i32 addrspace(200)*), i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 4) to i32 addrspace(200)*), i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)], align [[$CAP_SIZE]]
// CHECK-LINKER-NOT: __cxx_global_var_init
