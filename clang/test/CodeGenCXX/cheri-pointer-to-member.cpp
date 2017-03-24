// RUN: %clang %s -mabi=purecap -fno-rtti -std=c++11 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s

class A {
public:
  int x = 3;
  int y = 4;
  int foo() { return 1; }
  virtual int foo_virtual() { return 2; }
  int bar() { return 1; }
  virtual int bar_virtual() { return 2; }
};

// compare IR with simulated function pointers:
struct mem_fn_ptr {
  void* ptr;
  long offset;
};
void func(void) {}
mem_fn_ptr virt = { (void*)32, 1 };
mem_fn_ptr nonvirt = { (void*)&func, 1 };
// CHECK: @virt = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 32 to i8 addrspace(200)*), i64 1 }, align 32
// CHECK: @nonvirt = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @_Z4funcv to i8*) to i8 addrspace(200)*), i64 1 }, align 32

// now the real thing
// FIXME: why is alignment only 8 here....

typedef int (A::* AMemberFuncPtr)();

AMemberFuncPtr global_null_func_ptr = nullptr;
int A::* global_data_ptr = &A::y;
AMemberFuncPtr global_nonvirt_func_ptr = &A::bar;
AMemberFuncPtr global_virt_func_ptr = &A::bar_virtual;
// FIXME: align should be 32...
// CHECK: @global_null_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } zeroinitializer, align
// CHECK: @global_data_ptr = addrspace(200) global i64 36, align 8
// CHECK: @global_nonvirt_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A3barEv to i8*) to i8 addrspace(200)*), i64 0 }, align
// CHECK: @global_virt_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 32 to i8 addrspace(200)*), i64 1 }, align


int main() {
  // CHECK: %data_ptr = alloca i64, align 8
  // CHECK-NOT: %func_ptr = alloca { i64, i64 }, align 8
  // CHECK-NOT: %virtual_func_ptr = alloca { i64, i64 }, align 8
  A a;
  // FIXME: alignment is wrong
  int A::* null_data_ptr = nullptr;
  // CHECK: store i64 -1, i64 addrspace(200)* %null_data_ptr, align 8
  int A::* data_ptr = &A::x;
  // CHECK: store i64 32, i64 addrspace(200)* %data_ptr, align 8
  int A::* data_ptr_2 = &A::y;
  // CHECK: store i64 36, i64 addrspace(200)* %data_ptr_2, align 8

  AMemberFuncPtr null_func_ptr = nullptr;
  // CHECK:   store { i8 addrspace(200)*, i64 } zeroinitializer, { i8 addrspace(200)*, i64 } addrspace(200)* %null_func_ptr, align 8
  AMemberFuncPtr func_ptr = &A::foo;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A3fooEv to i8*) to i8 addrspace(200)*), i64 0 }, { i8 addrspace(200)*, i64 } addrspace(200)* %func_ptr, align 8
  AMemberFuncPtr func_ptr_2 = &A::bar;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A3barEv to i8*) to i8 addrspace(200)*), i64 0 }, { i8 addrspace(200)*, i64 } addrspace(200)* %func_ptr_2, align 8
  AMemberFuncPtr virtual_func_ptr = &A::foo_virtual;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* null, i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* %virtual_func_ptr, align 8
   AMemberFuncPtr virtual_func_ptr_2 = &A::bar_virtual;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 32 to i8 addrspace(200)*), i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* %virtual_func_ptr_2, align 8

  bool dataptr_is_null = data_ptr == nullptr;
  // CHECK: %0 = load i64, i64 addrspace(200)* %data_ptr, align 8
  // CHECK-NEXT: %1 = icmp eq i64 %0, -1

#if 0
  bool funcptr_is_null = !func_ptr;
  bool virtual_func_ptr_is_null = !virtual_func_ptr;

  bool datacmp = data_ptr == data_ptr_2;
  bool funccmp = func_ptr == func_ptr_2;
  bool vfunccmp = virtual_func_ptr == virtual_func_ptr_2;
  bool vfunc_func_cmp = virtual_func_ptr == func_ptr;
#endif

  // return a.*data_ptr + (a.*func_ptr)() + (a.*virtual_func_ptr)();
  // return null_func_ptr == nullptr;
  return a.*data_ptr;
}


// taken from temporaries.cpp
namespace PR7556 {
  struct A { ~A(); };
  struct B { int i; ~B(); };
  struct C { int C::*pm; ~C(); };
  // xCHECK-LABEL: define void @_ZN6PR75563fooEv()
  void foo() {
    // xCHECK: call void @_ZN6PR75561AD1Ev
    A();
    // xCHECK: call void @llvm.memset.p200i8.i64
    // xCHECK: call void @_ZN6PR75561BD1Ev
    B();
    // xCHECK-NOT: llvm.memcpy.p200i8.p0i8.i64
    // xCHECK: call void @llvm.memcpy.p200i8.p200i8.i64
    // xCHECK: call void @_ZN6PR75561CD1Ev
    C();
    // xCHECK-NEXT: ret void
  }
}
