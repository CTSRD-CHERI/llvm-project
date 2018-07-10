// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy  -emit-llvm -o - %s | FileCheck %s -check-prefix CHECK
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel  -emit-llvm -o - %s | FileCheck %s -check-prefix CHECK

void asctime_r(buf)
char * buf;
{
}

void asctime()
{
  // CHECK: call void @asctime_r(i8 addrspace(200)* null)
  asctime_r(0);
}
