// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s
// Check that we can actually compile this file...
// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o /dev/null -c

// CHECK: c1
int c1(__capability void* x, __capability void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt i8 addrspace(200)*
  return a < b;
}
// CHECK c2
int c2(__capability void* x, __capability void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp eq i8 addrspace(200)*
  return a == b;
}
// CHECK: c3
int c3(__capability void* x, __capability void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sgt i8 addrspace(200)*
  return a > b;
}
// CHECK: c4
int c4(__capability void* x, __capability void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sge i8 addrspace(200)*
  return a >= b;
}
// CHECK: c5
int c5(__capability void* x, __capability void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sle i8 addrspace(200)*
  return a <= b;
}

// CHECK: ca1
int ca1(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: sub
  return a - b;
}

// CHECK: ca2
int ca2(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: add
  return a + b;
}

// CHECK: ca3
int ca3(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: mul
  return a * b;
}

// CHECK: ca4
int ca4(__capability void* x, __capability void* y)
{
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %x)
  __intcap_t a = (__intcap_t)x;
  // CHECK: @llvm.mips.cap.offset.get(i8 addrspace(200)* %y)
  __intcap_t b = (__intcap_t)y;
  // CHECK: sdiv
  return a / b;
}

// CHECK: p1
int p1(void* x, void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt i8 addrspace(200)*
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

// increment and decrement should work
__intcap_t x;
__uintcap_t y;

void incdec(void)
{
  x++;
  y++;
  x--;
  y--;
}

__uintcap_t xor(__uintcap_t f)
{
  f ^= 2;
  return f;
}

int capdiff(__capability int *a, __capability int *b)
{
  // CHECK: @capdiff(i32 addrspace(200)* %a, i32 addrspace(200)* %b) #0 {
  // CHECK: %0 = bitcast i32 addrspace(200)* %a to i8 addrspace(200)*
  // CHECK: %1 = bitcast i32 addrspace(200)* %b to i8 addrspace(200)*
  // CHECK: %2 = tail call i64 @llvm.mips.cap.base.get(i8 addrspace(200)* %0)
  // CHECK: %3 = tail call i64 @llvm.mips.cap.base.get(i8 addrspace(200)* %1)
  // CHECK: %4 = tail call i64 @llvm.mips.cap.offset.get(i8 addrspace(200)* %0)
  // CHECK: %5 = tail call i64 @llvm.mips.cap.offset.get(i8 addrspace(200)* %1)
  // CHECK: %sub.ptr.rhs.cast.neg = sub i64 %2, %3
  // CHECK: %sub.ptr.lhs.cast = add i64 %sub.ptr.rhs.cast.neg, %4
  // CHECK: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %5
  // CHECK: %sub.ptr.div.1 = lshr exact i64 %sub.ptr.sub, 2
  return a-b;
}

