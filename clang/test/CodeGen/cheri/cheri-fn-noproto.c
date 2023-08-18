// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s -Wall -verify | FileCheck %s

void asctime_r(buf) // expected-warning{{a function definition without a prototype is deprecated in all versions}}
char * buf;
{
}

void asctime()
{
  // CHECK: call void @asctime_r(i8 addrspace(200)* noundef null)
  asctime_r(0);
}
