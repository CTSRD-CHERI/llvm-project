// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

struct _FILE;
typedef struct _FILE FILE;
int	 fprintf(FILE* __restrict, const char * __restrict, ...);

void __assert() {
  // CHECK: call i32 (%struct._FILE addrspace(200)*, i8 addrspace(200)*, ...) @fprintf(%struct._FILE addrspace(200)* null, i8 addrspace(200)* getelementptr inbounds ([18 x i8], [18 x i8] addrspace(200)* @.str, i32 0, i32 0))
  fprintf(0, "Assertion failed:");
}
