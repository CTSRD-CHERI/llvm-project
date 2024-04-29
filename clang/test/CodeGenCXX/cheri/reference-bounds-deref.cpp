// Check that we can set bounds on references
// REQUIRES: asserts
// RUNNOT: %clang_cc1 -O0 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=address -emit-llvm -mllvm -debug
// RUNNOT: %clang_cc1 -O0 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=local-bounds,array-bounds,object-size -emit-llvm -mllvm -debug
// RUNNOT: %clang_cc1 -O2 -triple=x86_64-unknown-linux -fcolor-diagnostics %s -o - -fsanitize=local-bounds,array-bounds,object-size -emit-llvm
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -O2 -std=c++17 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds 2>%t.dbg -print-stats | FileCheck %s
// RUN: FileCheck %s -check-prefix DEBUG-MSG -input-file=%t.dbg

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
// CHECK-LABEL: define dso_local void @_Z10test_derefPi(ptr addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 4)

// Same with double
TEST_PTR_TO_REF(double)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'double' reference to 8
// CHECK-LABEL: define dso_local void @_Z10test_derefPd(ptr addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 8)

// Or void*
TEST_PTR_TO_REF(void *)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'void * __capability' reference to 16
// CHECK-LABEL: define dso_local void @_Z10test_derefPPv(ptr addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 16)

enum Enum1 { E1 };
// Enum size is known -> set bounds
TEST_PTR_TO_REF(Enum1)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'Enum1' reference to 4
// CHECK-LABEL: define dso_local void @_Z10test_derefP5Enum1(ptr addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 4)

enum class EnumClass : unsigned short { EC1 };
// Enum size is known -> set bounds
TEST_PTR_TO_REF(EnumClass)
// DEBUG-MSG-NEXT: Found scalar type -> setting bounds for 'EnumClass' reference to 2
// CHECK-LABEL: define dso_local void @_Z10test_derefP9EnumClass(ptr addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 2)

// No bounds here since Foo might be subclassed
TEST_PTR_TO_REF(Foo)
// DEBUG-MSG-NEXT: Found record type 'Foo' -> non-final class and using sub-object-safe mode -> not setting bounds
// CHECK-LABEL: declare void @_Z17do_stuff_with_refI3FooEvRT_(ptr addrspace(200)
// CHECK-NOT: @llvm.cheri.cap.bounds.set.i64

// Final class without vtable-> bounds
TEST_PTR_TO_REF(FinalClassNoVTable)
// DEBUG-MSG-NEXT: Found record type 'FinalClassNoVTable' -> is literal type and is marked as final -> setting bounds for 'FinalClassNoVTable' reference to 4
// CHECK-LABEL: define dso_local void @_Z10test_derefP18FinalClassNoVTable(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 4)

// FIXME: should be able to set bounds here
// We can set bounds here since there are no subclasses (even though it has a vtable)
TEST_PTR_TO_REF(FinalClassWithVTable)
// DEBUG-MSG-NEXT: Found record type 'FinalClassWithVTable' -> final but not a literal type -> size might by dynamic -> not setting bounds
// CHECK-LABEL: define dso_local void @_Z10test_derefP20FinalClassWithVTable(
// TODO-CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// TODO-CHECK-SAME: i64 1234)
// for now no bounds on classes with vtables
// CHECK-NOT: @llvm.cheri.cap.bounds.set.i64

// Final class (literal) no vtable -> bounds
TEST_PTR_TO_REF(FinalClassInheritedNoVTable)
// DEBUG-MSG-NEXT: Found record type 'FinalClassInheritedNoVTable' -> is literal type and is marked as final -> setting bounds for 'FinalClassInheritedNoVTable' reference to 12
// CHECK-LABEL: define dso_local void @_Z10test_derefP27FinalClassInheritedNoVTable(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 12)

// FIXME: should be able to set bounds here
TEST_PTR_TO_REF(FinalClassInheritedWithVTable)
// DEBUG-MSG-NEXT: Found record type 'FinalClassInheritedWithVTable' -> final but not a literal type -> size might by dynamic -> not setting bounds
// CHECK-LABEL: define dso_local void @_Z10test_derefP29FinalClassInheritedWithVTable(
// TODO-CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// TODO-CHECK-SAME: i64 1234)
// for now no bounds on classes with vtables
// CHECK-NOT: @llvm.cheri.cap.bounds.set.i64

// Shouldn't be able to set bounds here since there is a flexible array member
TEST_PTR_TO_REF(FlexArrayBase)
// DEBUG-MSG-NEXT: reference 'FlexArrayBase' subobj bounds check: Found record type 'FlexArrayBase' -> found real VLA in FlexArrayBase -> record has flexible array member -> setting bounds for 'FlexArrayBase' reference to remaining
// CHECK-LABEL: define dso_local void @_Z10test_derefP13FlexArrayBase(
// CHECK: @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}}, i64 %remaining_bytes)
TEST_PTR_TO_REF(FlexArrayFinal)
// DEBUG-MSG-NEXT: reference 'FlexArrayFinal' subobj bounds check: Found record type 'FlexArrayFinal' -> found real VLA in FlexArrayFinal -> record has flexible array member -> setting bounds for 'FlexArrayFinal' reference to remaining
// CHECK-LABEL: define dso_local void @_Z10test_derefP14FlexArrayFinal(
// CHECK: @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}}, i64 %remaining_bytes)

// No bounds on this struct (even though it is C-like) since it might have inheritors
// TODO: add a mode where we do add the bounds and require annotations?
TEST_PTR_TO_REF(CLike)
// DEBUG-MSG-NEXT: Found record type 'CLike' -> non-final class and using sub-object-safe mode -> not setting bounds
// CHECK-LABEL: define dso_local void @_Z10test_derefP5CLike(
// CHECK-NOT: @llvm.cheri.cap.bounds.set.i64
// Should have bounds here:
TEST_PTR_TO_REF(CLikeFinal)
// DEBUG-MSG-NEXT: Found record type 'CLikeFinal' -> is C-like struct type and is marked as final -> setting bounds for 'CLikeFinal' reference to 16
// CHECK-LABEL: define dso_local void @_Z10test_derefP10CLikeFinal(
// CHECK: call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %{{.+}},
// CHECK-SAME: i64 16)

union IntOrFloat {
  int i;
  float f;
};
TEST_PTR_TO_REF(IntOrFloat)
// DEBUG-MSG-NEXT: Found record type 'IntOrFloat' -> not a struct/class -> not setting bounds
// CHECK-LABEL: define dso_local void @_Z10test_derefP10IntOrFloat(
// CHECK-NOT: @llvm.cheri.cap.bounds.set.i64

// finally check the dumped statistics:
// DEBUG-MSG-LABEL: STATISTICS:
// DEBUG-MSG: ... Statistics Collected ...
// DEBUG-MSG: 15 cheri-bounds     - Number of references checked for tightening bounds
// DEBUG-MSG:  8 cheri-bounds     - Number of references where bounds were tightened
