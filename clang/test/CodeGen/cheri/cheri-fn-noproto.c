// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s -Wall -verify | FileCheck %s
// expected-no-diagnostics
void asctime_r(buf)
char * buf;
{
}

void asctime()
{
  // CHECK: call void @asctime_r(i8 addrspace(200)* noundef null)
  asctime_r(0);
}
