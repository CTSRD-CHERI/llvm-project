// REQUIRES: mips-registered-target
// RUN: %cheri_cc1 %s -O0 -o - -emit-llvm -cheri-uintcap=offset -verify=expected,offset | FileCheck %s -check-prefixes CHECK,OFFSET
// RUN: %cheri_cc1 %s -O0 -o - -emit-llvm -cheri-uintcap=addr -verify | FileCheck %s  -check-prefixes CHECK,ADDR
// Check that we can actually compile this file...
// RUN: %cheri_cc1 -emit-obj %s -O0 -o /dev/null

// CHECK-LABEL: i32 @c1(
int c1(void* __capability x, void* __capability y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp slt i8 addrspace(200)*
  return a < b;
}
// CHECK-LABEL: i32 @c2(
int c2(void* __capability x, void* __capability y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp eq i8 addrspace(200)*
  return a == b;
}
// CHECK-LABEL: i32 @c3(
int c3(void* __capability x, void* __capability y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp sgt i8 addrspace(200)*
  return a > b;
}
// CHECK-LABEL: i32 @c4(
int c4(void* __capability x, void* __capability y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp sge i8 addrspace(200)*
  return a >= b;
}

// CHECK-LABEL: i32 @c5(
int c5(void* __capability x, void* __capability y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp sle i8 addrspace(200)*
  return a <= b;
}

// CHECK-LABEL: i32 @ca1(
int ca1(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap a = (__intcap)x;
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap b = (__intcap)y;
  // CHECK: sub
  return a - b; // offset-warning{{subtracting '__intcap' from '__intcap' may give an unexpected result}}
  // offset-note@-1{{in order to perform pointer arithmetic, cast the right-hand oper}}
}

// CHECK-LABEL: i32 @ca2(
int ca2(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap a = (__intcap)x;
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap b = (__intcap)y;
  // CHECK: add
  return a + b; // expected-warning{{it is not clear which should be used as the source of provenance}}
}

// CHECK-LABEL: i32 @ca3(
int ca3(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap a = (__intcap)x;
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap b = (__intcap)y;
  // CHECK: mul
  return a * b; // expected-warning{{it is not clear which should be used as the source of provenance}}
}

// CHECK-LABEL: i32 @ca4(
int ca4(void* __capability x, void* __capability y)
{
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap a = (__intcap)x;
  // OFFSET: @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
  // ADDR: @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*
  __intcap b = (__intcap)y;
  // CHECK: sdiv
  return a / b;
}

// CHECK-LABEL: i32 @p1(
int p1(void* x, void* y)
{
  __intcap a = (__intcap)x;
  __intcap b = (__intcap)y;
  // CHECK: icmp slt i8 addrspace(200)*
  return a < b;
}

// CHECK-LABEL: @castc(
void * __capability castc(__intcap a)
{
  // CHECK-NOT: ptrtoint
  return (void* __capability)a;
}

// CHECK-LABEL: @castp(
void *castp(__intcap a)
{
  // CHECK: [[CTOTPR:%.+]] = call i64 @llvm.cheri.cap.to.pointer.i64(
  // CHECK: inttoptr i64 [[CTOTPR]] to i8*
  return (void*)a; // expected-warning{{the following conversion will result in a CToPtr operation}}
  // expected-note@-1{{if you really intended to use CToPtr use __builtin_cheri_cap_to_pointer()}}
}

// increment and decrement should work
__intcap x;
unsigned __intcap y;

void incdec(void)
{
  x++;
  y++;
  x--;
  y--;
}

unsigned __intcap xor(unsigned __intcap f)
{
  f ^= 2; // offset-warning{{using xor on a capability type only operates on the offset}}
  return f;
}

int capdiff(int * __capability a, int * __capability b)
{
  // CHECK-LABEL: @capdiff(i32 addrspace(200)*{{.*}}, i32 addrspace(200)*{{.*}})
  // CHECK: call i64 @llvm.cheri.cap.diff.i64(i8 addrspace(200)*
  // CHECK: %{{.*}} = trunc i64 %{{.*}} to i32
  return a-b;
}

// CHECK-LABEL: @negativeint(
void negativeint()
{
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 -5), i8 addrspace(200)** %minus
  __intcap minus = -5;
}

// CHECK-LABEL: @largeint(
void largeint()
{
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 4294967295), i8 addrspace(200)** %large
  unsigned __intcap large = 4294967295; // 2^32 - 1
}
