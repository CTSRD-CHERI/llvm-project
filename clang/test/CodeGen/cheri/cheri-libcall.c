// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

struct _FILE;
typedef struct _FILE FILE;
int	 fprintf(FILE* __restrict, const char * __restrict, ...);

void __assert() {
  // CHECK: call signext i32 (%struct._FILE addrspace(200)*, i8 addrspace(200)*, ...) @fprintf(%struct._FILE addrspace(200)* noundef null, i8 addrspace(200)* noundef getelementptr inbounds ([18 x i8], [18 x i8] addrspace(200)* @.str, i64 0, i64 0))
  fprintf(0, "Assertion failed:");
}
