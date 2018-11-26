// RUN: %cheri_cc1 %s -O0 -o - -emit-llvm -cheri-uintcap=offset | FileCheck %s -check-prefixes CHECK,OFFSET
// RUN: %cheri_cc1 %s -O0 -o - -emit-llvm -cheri-uintcap=addr | FileCheck %s  -check-prefixes CHECK,ADDR
// Check that we can actually compile this file...
// RUN: %cheri_cc1 -emit-obj %s -O0 -o /dev/null

// CHECK-LABEL: i32 @c1(
int c1(void* __capability x, void* __capability y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt i8 addrspace(200)*
  return a < b;
}
// CHECK-LABEL: i32 @c2(
int c2(void* __capability x, void* __capability y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp eq i8 addrspace(200)*
  return a == b;
}
// CHECK-LABEL: i32 @c3(
int c3(void* __capability x, void* __capability y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sgt i8 addrspace(200)*
  return a > b;
}
// CHECK-LABEL: i32 @c4(
int c4(void* __capability x, void* __capability y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sge i8 addrspace(200)*
  return a >= b;
}

// CHECK-LABEL: i32 @c5(
int c5(void* __capability x, void* __capability y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp sle i8 addrspace(200)*
  return a <= b;
}

// CHECK-LABEL: i32 @ca1(
int ca1(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t a = (__intcap_t)x;
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t b = (__intcap_t)y;
  // CHECK: sub
  return a - b;
}

// CHECK-LABEL: i32 @ca2(
int ca2(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t a = (__intcap_t)x;
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t b = (__intcap_t)y;
  // CHECK: add
  return a + b;
}

// CHECK-LABEL: i32 @ca3(
int ca3(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t a = (__intcap_t)x;
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t b = (__intcap_t)y;
  // CHECK: mul
  return a * b;
}

// CHECK-LABEL: i32 @ca4(
int ca4(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t a = (__intcap_t)x;
  // OFFSET: @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get(i8 addrspace(200)*
  __intcap_t b = (__intcap_t)y;
  // CHECK: sdiv
  return a / b;
}

// CHECK-LABEL: i32 @p1(
int p1(void* x, void* y)
{
  __intcap_t a = (__intcap_t)x;
  __intcap_t b = (__intcap_t)y;
  // CHECK: icmp slt i8 addrspace(200)*
  return a < b;
}

// CHECK-LABEL: @castc(
void * __capability castc(__intcap_t a)
{
  // CHECK-NOT: ptrtoint
  return (void* __capability)a;
}

// CHECK-LABEL: @castp(
void *castp(__intcap_t a)
{
  // CHECK: ptrtoint i8 addrspace(200)* 
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

int capdiff(int * __capability a, int * __capability b)
{
  // CHECK-LABEL: @capdiff(i32 addrspace(200)*{{.*}}, i32 addrspace(200)*{{.*}})
  // CHECK: call i64 @llvm.cheri.cap.diff(i8 addrspace(200)*
  // CHECK: %{{.*}} = trunc i64 %{{.*}} to i32
  return a-b;
}

// CHECK-LABEL: @negativeint(
void negativeint()
{
  // OFFSET: @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 -5)
  // ADDR: @llvm.cheri.cap.address.set(i8 addrspace(200)* null, i64 -5)
  __intcap_t minus = -5;
}

// CHECK-LABEL: @largeint(
void largeint()
{
  // OFFSET: @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 4294967295)
  // ADDR: @llvm.cheri.cap.address.set(i8 addrspace(200)* null, i64 4294967295)
  __uintcap_t large = 4294967295; // 2^32 - 1
}
