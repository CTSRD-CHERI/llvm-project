// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

void asctime_r(buf)
char * buf;
{
}

void asctime()
{
  // CHECK: call void addrspacecast (void (i8 addrspace(200)*)* @asctime_r to void (i8 addrspace(200)*) addrspace(200)*)(i8 addrspace(200)* null)
  asctime_r(0);
}
