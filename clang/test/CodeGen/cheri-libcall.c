// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -emit-llvm -S -o - %s | FileCheck %s

int	 fprintf(char* __restrict, const char * __restrict, ...);

void __assert() {
  // CHECK: %{{.*}} = bitcast i8 addrspace(200)* %{{.*}} to i32 (i8 addrspace(200)*, i8 addrspace(200)*, ...) addrspace(200)*
  // CHECK: call i32 (i8 addrspace(200)*, i8 addrspace(200)*, ...) %{{.*}}
  fprintf(0, "Assertion failed:");
}
