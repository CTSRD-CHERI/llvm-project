// Check that we can set bounds on array subscript references
// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -O2 -std=c++17 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds -print-stats 2>&1 | FileCheck %s

void do_stuff_with_ref(int&);
void test_array() {
  int intarray[2] = {1, 2};
  do_stuff_with_ref(intarray[0]);
  do_stuff_with_ref(intarray[1]);
  do_stuff_with_ref(intarray[2]);
// CHECK: Found scalar type -> setting bounds for 'int' reference to 4
// CHECK: Found scalar type -> setting bounds for 'int' reference to 4
// CHECK: Found scalar type -> setting bounds for 'int' reference to 4

}



class Foo {
public:
  int dummy;
  int payload;
};
void do_stuff_with_ref(Foo&);

void test_foo_array() {

  Foo foo_array[2] = { {1, 2}, {2, 3} };
  do_stuff_with_ref(foo_array[0]);
  do_stuff_with_ref(foo_array[1]);
  do_stuff_with_ref(foo_array[2]);
  // TODO: we should actually be able to set bounds here since it is stack allocated and we know the real type
  // CHECK: Found record type 'class Foo' -> not final -> can't assume it has no inheritors
  // CHECK: Found record type 'class Foo' -> not final -> can't assume it has no inheritors
  // CHECK: Found record type 'class Foo' -> not final -> can't assume it has no inheritors
}

struct Foo_final final {
  int x;
  int y;
};

void do_stuff_with_ref(Foo_final&);

void test_final_class_array() {
  Foo_final foo_array[2] = { {1, 2}, {2, 3} };
  do_stuff_with_ref(foo_array[0]);
  do_stuff_with_ref(foo_array[1]);
  do_stuff_with_ref(foo_array[2]);
  // CHECK: Found record type 'struct Foo_final' -> is C-like struct type and is marked as final -> setting bounds for 'struct Foo_final' reference to 8
  // CHECK: Found record type 'struct Foo_final' -> is C-like struct type and is marked as final -> setting bounds for 'struct Foo_final' reference to 8
  // CHECK: Found record type 'struct Foo_final' -> is C-like struct type and is marked as final -> setting bounds for 'struct Foo_final' reference to 8
}


// finally check the dumped statistics:
// CHECK-LABEL: STATISTICS:
// CHECK: ... Statistics Collected ...
// CHECK:  6 cheri-bounds     - Number of references where bounds were tightend
// CHECK:  9 cheri-bounds     - Number of references processed for tightening bounds
