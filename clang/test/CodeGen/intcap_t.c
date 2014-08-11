// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s

// CHECK: c1
int c1(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x
  __intcap_t a = (__intcap_t)x;
  // CHECK: ptrtoint i8 addrspace(200)* %y
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt
  return a < b;
}
// CHECK c2
int c2(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x
  __intcap_t a = (__intcap_t)x;
  // CHECK: ptrtoint i8 addrspace(200)* %y
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp eq
  return a == b;
}
// CHECK: c3
int c3(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x
  __intcap_t a = (__intcap_t)x;
  // CHECK: ptrtoint i8 addrspace(200)* %y
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sgt
  return a > b;
}
// CHECK: c4
int c4(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x
  __intcap_t a = (__intcap_t)x;
  // CHECK: ptrtoint i8 addrspace(200)* %y
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sge
  return a >= b;
}
// CHECK: c5
int c5(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x
  __intcap_t a = (__intcap_t)x;
  // CHECK: ptrtoint i8 addrspace(200)* %y
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sle
  return a <= b;
}

// CHECK: ca1
int ca1(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: sub
  return a - b;
}

// CHECK: ca2
int ca2(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: add
  return a + b;
}

// CHECK: ca3
int ca3(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: mul
  return a * b;
}

// CHECK: ca4
int ca4(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.get.offset(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: sdiv
  return a / b;
}

// CHECK: p1
int p1(void* x, void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt
  return a < b;
}

// CHECK: castc
__capability void *castc(__intcap_t a)
{
  // CHECK-NOT: ptrtoint
  return (__capability void*)a;
}

void *castp(__intcap_t a)
{
  // CHECK: ptrtoint i8 addrspace(200)* %a to i64
  // CHECK: inttoptr
  return (void*)a;
}

// Simplify casts bug:
int cp(__capability void* x, __capability void* y)
{
  // CHECK: ptrtoint i8 addrspace(200)* %x to i64
  // CHECK: ptrtoint i8 addrspace(200)* %y to i64
  return x - y;
}

