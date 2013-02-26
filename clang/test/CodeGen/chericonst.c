// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s


int cst(const __capability void *x);

int callconst(void)
{
  struct x { int a,b,c; } x;
  __capability struct x *y = (__capability struct x*)&x;
  // Make sure we remove the write permissions from the capability
  // CHECK: call i8 addrspace(200)* @llvm.cheri.and.cap.perms(i8 addrspace(200)* %2, i64 65495)
  cst(y);
}

