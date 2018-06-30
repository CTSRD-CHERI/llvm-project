// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -O0 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -O3 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s | FileCheck %s -check-prefix OPT
// Check that it doesn't crash when compiling:
// RUN: %cheri_purecap_cc1 -S -mrelocation-model pic -pic-level 1 -O2 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o /dev/null %s

// The optimize libcalls pass changes the global string from AS200 to AS0
// This happens when in converts printf to puts

void a(void) {
  printf("Hello\n");
  // CHECK: @.str = private unnamed_addr addrspace(200) constant [7 x i8] c"Hello\0A\00"
  // CHECK: call i32 (i8 addrspace(200)*, ...) @printf(i8 addrspace(200)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(200)* @.str
  // OPT: @str = private unnamed_addr addrspace(200) constant [6 x i8] c"Hello\00"
  // OPT: call {{.+}} @puts({{.+}} [6 x i8] addrspace(200)* @str
}
