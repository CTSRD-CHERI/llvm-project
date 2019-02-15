// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s
// XXXRW: This functionality is disabled due to source-code compatibility issues.
// XFAIL: *


int cst(const __capability void *x);

int callconst(void)
{
  struct x { int a,b,c; } x;
  __capability struct x *y = (__capability struct x*)&x;
  // Make sure we remove the write permissions from the capability
  // CHECK: call i8 addrspace(200)* @llvm.mips.and.cap.perms(
  // CHECK: i64 65495)
  cst(y);
  return 0;
}

