// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

void asctime_r(buf)
char * buf;
{
}

void asctime()
{
  // CHECK: call void @asctime_r(i8 addrspace(200)* null)
  asctime_r(0);
}
