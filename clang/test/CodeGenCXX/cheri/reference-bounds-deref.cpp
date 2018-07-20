// Check that we can set bounds on references
// REQUIRES: asserts
// RUNNOT: %clang_cc1 -O0 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=address -emit-llvm -mllvm -debug
// RUNNOT: %clang_cc1 -O0 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=local-bounds,array-bounds,object-size -emit-llvm -mllvm -debug
// RUNNOT: %clang_cc1 -O2 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=local-bounds,array-bounds,object-size -emit-llvm
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -O2 -std=c++17 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds 2>%t.dbg -print-stats | %cheri_FileCheck %s
// RUN: %cheri_FileCheck %s -check-prefix DEBUG-MSG < %t.dbg

class Foo {
public:
  int dummy;
  int payload;
};

class FinalClassNoVTable final {
public:
  void foo(){};
  int i;
};

class FinalClassWithVTable final {
public:
  virtual void foo(){};
  int i;
};

class FinalClassInheritedNoVTable final : public Foo {
public:
  int i;
};

class FinalClassInheritedWithVTable final : public Foo {
public:
  virtual void foo(){};
  int i;
};

struct FlexArrayBase {
  int i;
  char flexarray[];
};

#if 0 // Not allowed in C++
struct FlexArrayInheritor final: public FlexArrayBase {
  int i;
};
#endif

struct FlexArrayFinal final {
  int i;
  char flexarray[];
};

struct CLike {
  int first;
  int second;
  int third;
};

struct CLikeFinal final {
  int data[4];
};

// FIXME: multiple inheritance..... and virtual inheritance

template <typename T>
void do_stuff_with_ref(T &ref);

#define TEST_PTR_TO_REF(type)  \
  void test_deref(type *ptr) { \
    do_stuff_with_ref(*ptr);   \
  }

// Should be able to set bounds on int
TEST_PTR_TO_REF(int)
// DEBUG-MSG: Found scalar type -> setting bounds for 'int' reference to 4
// CHECK-LABEL: define void @_Z10test_derefU3capPi(i32 addrspace(200)*
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 4)

// Same with double
TEST_PTR_TO_REF(double)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'double' reference to 8
// CHECK-LABEL: define void @_Z10test_derefU3capPd(double addrspace(200)*
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 8)

// Or void*
TEST_PTR_TO_REF(void *)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'void * __capability' reference to [[$CAP_SIZE]]
// CHECK-LABEL: define void @_Z10test_derefU3capPU3capPv(i8 addrspace(200)* addrspace(200)*
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 [[$CAP_SIZE]])

enum Enum1 { E1 };
// Enum size is known -> set bounds
TEST_PTR_TO_REF(Enum1)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'enum Enum1' reference to 4
// CHECK-LABEL: define void @_Z10test_derefU3capP5Enum1(i32 addrspace(200)*
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 4)

enum class EnumClass : unsigned short { EC1 };
// Enum size is known -> set bounds
TEST_PTR_TO_REF(EnumClass)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'enum EnumClass' reference to 2
// CHECK-LABEL: define void @_Z10test_derefU3capP9EnumClass(i16 addrspace(200)*
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 2)

// No bounds here since Foo might be subclassed
TEST_PTR_TO_REF(Foo)
// DEBUG-MSG-NEXT: Found record type 'class Foo' -> not final -> can't assume it has no inheritors
// CHECK-LABEL: declare void @_Z17do_stuff_with_refI3FooEvU3capRT_(%class.Foo addrspace(200)*
// CHECK-NOT: @llvm.cheri.cap.bounds.set

// Final class without vtable-> bounds
TEST_PTR_TO_REF(FinalClassNoVTable)
// DEBUG-MSG-NEXT: Found record type 'class FinalClassNoVTable' -> is literal type and is marked as final -> setting bounds for 'class FinalClassNoVTable' reference to 4
// CHECK-LABEL: define void @_Z10test_derefU3capP18FinalClassNoVTable(
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 4)

// FIXME: should be able to set bounds here
// We can set bounds here since there are no subclasses (even though it has a vtable)
TEST_PTR_TO_REF(FinalClassWithVTable)
// DEBUG-MSG-NEXT: Found record type 'class FinalClassWithVTable' -> final but not a literal type -> size might by dynamic -> not setting bounds
// CHECK-LABEL: define void @_Z10test_derefU3capP20FinalClassWithVTable(
// TODO-CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// TODO-CHECK-SAME: i64 1234)
// for now no bounds on classes with vtables
// CHECK-NOT: @llvm.cheri.cap.bounds.set

// Final class (literal) no vtable -> bounds
TEST_PTR_TO_REF(FinalClassInheritedNoVTable)
// DEBUG-MSG-NEXT: Found record type 'class FinalClassInheritedNoVTable' -> is literal type and is marked as final -> setting bounds for 'class FinalClassInheritedNoVTable' reference to 12
// CHECK-LABEL: define void @_Z10test_derefU3capP27FinalClassInheritedNoVTable(
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 12)

// FIXME: should be able to set bounds here
TEST_PTR_TO_REF(FinalClassInheritedWithVTable)
// DEBUG-MSG-NEXT: Found record type 'class FinalClassInheritedWithVTable' -> final but not a literal type -> size might by dynamic -> not setting bounds
// CHECK-LABEL: define void @_Z10test_derefU3capP29FinalClassInheritedWithVTable(
// TODO-CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// TODO-CHECK-SAME: i64 1234)
// for now no bounds on classes with vtables
// CHECK-NOT: @llvm.cheri.cap.bounds.set

// Shouldn't be able to set bounds here since there is a flexible array member
TEST_PTR_TO_REF(FlexArrayBase)
// DEBUG-MSG-NEXT: Found record type 'struct FlexArrayBase' -> has flexible array member -> can't set bounds
// CHECK-LABEL: define void @_Z10test_derefU3capP13FlexArrayBase(
// CHECK-NOT: @llvm.cheri.cap.bounds.set
TEST_PTR_TO_REF(FlexArrayFinal)
// DEBUG-MSG-NEXT: Found record type 'struct FlexArrayFinal' -> has flexible array member -> can't set bounds
// CHECK-LABEL: define void @_Z10test_derefU3capP14FlexArrayFinal(
// CHECK-NOT: @llvm.cheri.cap.bounds.set

// No bounds on this struct (even though it is C-like) since it might have inheritors
// TODO: add a mode where we do add the bounds and require annotations?
TEST_PTR_TO_REF(CLike)
// DEBUG-MSG-NEXT: Found record type 'struct CLike' -> not final -> can't assume it has no inheritors
// CHECK-LABEL: define void @_Z10test_derefU3capP5CLike(
// CHECK-NOT: @llvm.cheri.cap.bounds.set
// Should have bounds here:
TEST_PTR_TO_REF(CLikeFinal)
// DEBUG-MSG-NEXT: Found record type 'struct CLikeFinal' -> is C-like struct type and is marked as final -> setting bounds for 'struct CLikeFinal' reference to 16
// CHECK-LABEL: define void @_Z10test_derefU3capP10CLikeFinal(
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}},
// CHECK-SAME: i64 16)

union IntOrFloat {
  int i;
  float f;
};
TEST_PTR_TO_REF(IntOrFloat)
// DEBUG-MSG-NEXT: Found record type 'union IntOrFloat' -> not a struct/class -> not setting bounds
// CHECK-LABEL: define void @_Z10test_derefU3capP10IntOrFloat(
// CHECK-NOT: @llvm.cheri.cap.bounds.set

// finally check the dumped statistics:
// DEBUG-MSG-LABEL: STATISTICS:
// DEBUG-MSG: ... Statistics Collected ...
// DEBUG-MSG:  8 cheri-bounds     - Number of references where bounds were tightend
// DEBUG-MSG: 15 cheri-bounds     - Number of references checked for tightening bounds
