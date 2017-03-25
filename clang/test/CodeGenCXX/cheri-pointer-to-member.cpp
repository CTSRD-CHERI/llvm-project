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

#if 0
  bool datacmp = data_ptr == data_ptr_2;
  bool funccmp = func_ptr == func_ptr_2;
  bool vfunccmp = virtual_func_ptr == virtual_func_ptr_2;
  bool vfunc_func_cmp = virtual_func_ptr == func_ptr;
#endif

  // return a.*data_ptr + (a.*func_ptr)() + (a.*virtual_func_ptr)();
  // return null_func_ptr == nullptr;
  return a.*data_ptr;
}

bool data_ptr_is_nonnull(int A::* ptr) {
  return static_cast<bool>(ptr);
  // CHECK: define zeroext i1 @_Z19data_ptr_is_nonnullM1Ai(i64 %ptr)
  // CHECK: %0 = load i64, i64 addrspace(200)* %ptr.addr, align 8
  // CHECK: %memptr.tobool = icmp ne i64 %0, -1
  // CHECK: ret i1 %memptr.tobool
}

bool data_ptr_is_null(int A::* ptr) {
  // CHECK: define zeroext i1 @_Z16data_ptr_is_nullM1Ai(i64 %ptr)
  // CHECK: %0 = load i64, i64 addrspace(200)* %ptr.addr, align 8
  // CHECK: %memptr.tobool = icmp ne i64 %0, -1
  // CHECK: %lnot = xor i1 %memptr.tobool, true
  // CHECK: ret i1 %lnot
  return !ptr;
}

// TODO: this could be simplified to test the tag bit of the address instead
// of checking the low bit of the adjustment

bool func_ptr_is_nonnull() {
  // FIXME: member pointers are not being passed correctly as arguments (probably because ismemcaptype returns false?)
  AMemberFuncPtr ptr = &A::bar;
  return static_cast<bool>(ptr);
  // CHECK: define zeroext i1 @_Z19func_ptr_is_nonnullv() #0 {
  // CHECK: %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %0, 0
  // CHECK: %memptr.tobool = icmp ne i8 addrspace(200)* %memptr.ptr, null
  // CHECK: %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %0, 1
  // CHECK: %memptr.virtualbit = and i64 %memptr.adj, 1
  // CHECK: %memptr.isvirtual = icmp ne i64 %memptr.virtualbit, 0
  // CHECK: %memptr.isnonnull = or i1 %memptr.tobool, %memptr.isvirtual
  // CHECK: ret i1 %memptr.isnonnull
}

bool func_ptr_is_null() {
  // FIXME: member pointers are not being passed correctly as arguments (probably because ismemcaptype returns false?)
  AMemberFuncPtr ptr = &A::foo_virtual;
  return !ptr;
  // CHECK: define zeroext i1 @_Z16func_ptr_is_nullv() #0 {
  // CHECK: %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %0, 0
  // CHECK: %memptr.tobool = icmp ne i8 addrspace(200)* %memptr.ptr, null
  // CHECK: %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %0, 1
  // CHECK: %memptr.virtualbit = and i64 %memptr.adj, 1
  // CHECK: %memptr.isvirtual = icmp ne i64 %memptr.virtualbit, 0
  // CHECK: %memptr.isnonnull = or i1 %memptr.tobool, %memptr.isvirtual
  // CHECK: %lnot = xor i1 %memptr.isnonnull, true
  // CHECK: ret i1 %lnot
}

bool data_ptr_equal(int A::* ptr1, int A::* ptr2) {
  return ptr1 == ptr2;
  // CHECK: define zeroext i1 @_Z14data_ptr_equalM1AiS0_(i64 %ptr1, i64 %ptr2)
  // CHECK: %0 = load i64, i64 addrspace(200)* %ptr1.addr, align 8
  // CHECK: %1 = load i64, i64 addrspace(200)* %ptr2.addr, align 8
  // CHECK: %2 = icmp eq i64 %0, %1
  // CHECK: ret i1 %2
}

bool data_ptr_not_equal(int A::* ptr1, int A::* ptr2) {
  return ptr1 != ptr2;
  // CHECK: define zeroext i1 @_Z18data_ptr_not_equalM1AiS0_(i64 %ptr1, i64 %ptr2)
  // CHECK: %0 = load i64, i64 addrspace(200)* %ptr1.addr, align 8
  // CHECK: %1 = load i64, i64 addrspace(200)* %ptr2.addr, align 8
  // CHECK: %2 = icmp ne i64 %0, %1
  // CHECK: ret i1 %2
}

int data_ptr_dereferece(A* a, int A::* ptr) {
  return a->*ptr;
  // CHECK: define i32 @_Z19data_ptr_derefereceP1AMS_i(%class.A addrspace(200)* %a, i64 %ptr)
  // CHECK: %0 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  // CHECK: %1 = load i64, i64 addrspace(200)* %ptr.addr, align 8
  // CHECK: %2 = bitcast %class.A addrspace(200)* %0 to i8 addrspace(200)*
  // CHECK: %memptr.offset = getelementptr inbounds i8, i8 addrspace(200)* %2, i64 %1
  // CHECK: %3 = bitcast i8 addrspace(200)* %memptr.offset to i32 addrspace(200)*
  // CHECK: %4 = load i32, i32 addrspace(200)* %3, align 4
  // CHECK: ret i32 %4
}

bool func_ptr_equal() {
  // FIXME: member pointers are not being passed correctly as arguments (probably because ismemcaptype returns false?)
  AMemberFuncPtr ptr1 = &A::foo_virtual;
  AMemberFuncPtr ptr2 = &A::bar;
  return ptr1 == ptr2;
  // CHECK: define zeroext i1 @_Z14func_ptr_equalv() #0 {
  // CHECK: %lhs.memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %0, 0
  // CHECK: %rhs.memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %1, 0
  // CHECK: %cmp.ptr = icmp eq i8 addrspace(200)* %lhs.memptr.ptr, %rhs.memptr.ptr
  // CHECK: %cmp.ptr.null = icmp eq i8 addrspace(200)* %lhs.memptr.ptr, null
  // CHECK: %lhs.memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %0, 1
  // CHECK: %rhs.memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  // CHECK: %cmp.adj = icmp eq i64 %lhs.memptr.adj, %rhs.memptr.adj
  // CHECK: %or.adj = or i64 %lhs.memptr.adj, %rhs.memptr.adj
  // CHECK: %2 = and i64 %or.adj, 1
  // CHECK: %cmp.or.adj = icmp eq i64 %2, 0
  // CHECK: %3 = and i1 %cmp.ptr.null, %cmp.or.adj
  // CHECK: %4 = or i1 %3, %cmp.adj
  // CHECK: %memptr.eq = and i1 %cmp.ptr, %4
  // CHECK: ret i1 %memptr.eq
}

bool func_ptr_not_equal() {
  // FIXME: member pointers are not being passed correctly as arguments (probably because ismemcaptype returns false?)
  AMemberFuncPtr ptr1 = &A::foo_virtual;
  AMemberFuncPtr ptr2 = &A::bar;
  return ptr1 != ptr2;
  // CHECK: define zeroext i1 @_Z18func_ptr_not_equalv()
  // CHECK: %cmp.ptr = icmp ne i8 addrspace(200)* %lhs.memptr.ptr, %rhs.memptr.ptr
  // CHECK: %cmp.ptr.null = icmp ne i8 addrspace(200)* %lhs.memptr.ptr, null
  // CHECK: %lhs.memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %0, 1
  // CHECK: %rhs.memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  // CHECK: %cmp.adj = icmp ne i64 %lhs.memptr.adj, %rhs.memptr.adj
  // CHECK: %or.adj = or i64 %lhs.memptr.adj, %rhs.memptr.adj
  // CHECK: %2 = and i64 %or.adj, 1
  // CHECK: %cmp.or.adj = icmp ne i64 %2, 0
  // CHECK: %3 = or i1 %cmp.ptr.null, %cmp.or.adj
  // CHECK: %4 = and i1 %3, %cmp.adj
  // CHECK: %memptr.ne = or i1 %cmp.ptr, %4
  // CHECK: ret i1 %memptr.ne
}

int func_ptr_dereference(A* a) {
  // FIXME: member pointers are not being passed correctly as arguments (probably because ismemcaptype returns false?)
  AMemberFuncPtr ptr = &A::foo_virtual;
  return (a->*ptr)();
  // CHECK: define i32 @_Z20func_ptr_dereferenceP1A(%class.A addrspace(200)* %a)
  // CHECK: %0 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  // CHECK: %1 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %ptr, align 8
  // CHECK: %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  // CHECK: %memptr.adj.shifted = ashr i64 %memptr.adj, 1
  // CHECK: %2 = bitcast %class.A addrspace(200)* %0 to i8 addrspace(200)*
  // CHECK: %3 = getelementptr inbounds i8, i8 addrspace(200)* %2, i64 %memptr.adj.shifted
  // CHECK: %this.adjusted = bitcast i8 addrspace(200)* %3 to %class.A addrspace(200)*
  // CHECK: %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %1, 0
  // CHECK: %4 = and i64 %memptr.adj, 1
  // CHECK: %memptr.isvirtual = icmp ne i64 %4, 0
  // CHECK: br i1 %memptr.isvirtual, label %memptr.virtual, label %memptr.nonvirtual

  // CHECK: memptr.virtual:                                   ; preds = %entry
  // CHECK: %5 = bitcast %class.A addrspace(200)* %this.adjusted to i8 addrspace(200)* addrspace(200)*
  // CHECK: %vtable = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %5, align 32
  // CHECK: %memptr.vtable-offset = ptrtoint i8 addrspace(200)* %memptr.ptr to i64
  // CHECK: %6 = getelementptr i8, i8 addrspace(200)* %vtable, i64 %memptr.vtable-offset
  // CHECK: %7 = bitcast i8 addrspace(200)* %6 to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  // CHECK: %memptr.virtualfn = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* %7, align 32
  // CHECK: br label %memptr.end

  // CHECK: memptr.nonvirtual:                                ; preds = %entry
  // CHECK: %memptr.nonvirtualfn = bitcast i8 addrspace(200)* %memptr.ptr to i32 (%class.A addrspace(200)*) addrspace(200)*
  // CHECK: br label %memptr.end

  // CHECK: memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  // CHECK: %8 = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  // CHECK: %call = call i32 %8(%class.A addrspace(200)* %this.adjusted)
  // CHECK: ret i32 %call
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
