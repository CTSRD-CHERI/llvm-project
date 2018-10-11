// Check that the printf -> puts optimization doesn't introduce AS0 strings
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o - -O2 -emit-llvm %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -o - -O2 -emit-llvm %s | FileCheck %s

// CHECK: @str = private unnamed_addr addrspace(200) constant [14 x i8] c"Hello, world!\00"

extern int printf(const char*, ...);

void foo(void) {
  printf("Hello, world!\n");
  // CHECK: tail call i32 @puts(i8 addrspace(200)* getelementptr inbounds ([14 x i8], [14 x i8] addrspace(200)* @str, i64 0, i64 0))
}

