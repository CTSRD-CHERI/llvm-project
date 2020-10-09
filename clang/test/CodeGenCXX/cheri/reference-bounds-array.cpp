// Check that we can set bounds on array subscript references
// REQUIRES: asserts
// RUN: rm -f %t.stats
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -O2 -std=c++17 -emit-llvm %s -o /dev/null \
// RUN:    -Wno-array-bounds -Rcheri-subobject-bounds -verify \
// RUN:    -stats-file=%t.stats -mllvm -stats
// RUN: FileCheck -input-file=%t.stats %s

void do_stuff_with_ref(int&);
void test_array() {
  int intarray[2] = {1, 2};
  do_stuff_with_ref(intarray[0]); // expected-remark{{setting sub-object bounds for reference to 'int' to 4 bytes}}
  do_stuff_with_ref(intarray[1]); // expected-remark{{setting sub-object bounds for reference to 'int' to 4 bytes}}
  do_stuff_with_ref(intarray[2]); // expected-remark{{setting sub-object bounds for reference to 'int' to 4 bytes}}
}



class Foo {
public:
  int dummy;
  int payload;
};
void do_stuff_with_ref(Foo&);

void test_foo_array() {

  Foo foo_array[2] = { {1, 2}, {2, 3} };
  do_stuff_with_ref(foo_array[0]); // expected-remark{{setting sub-object bounds for reference to 'Foo' to 8 bytes}}
  do_stuff_with_ref(foo_array[1]); // expected-remark{{setting sub-object bounds for reference to 'Foo' to 8 bytes}}
  do_stuff_with_ref(foo_array[2]); // expected-remark{{setting sub-object bounds for reference to 'Foo' to 8 bytes}}
}

void test_foo_array_deref_pointer() {
  // We can't set bounds here since we don't know whether it is actually a Foo* or a subclass
  Foo* foo_array[2];
  do_stuff_with_ref(*foo_array[0]); // expected-remark {{not setting bounds for reference to 'Foo' (non-final class and using sub-object-safe mode)}}
  do_stuff_with_ref(*foo_array[1]); // expected-remark {{not setting bounds for reference to 'Foo' (non-final class and using sub-object-safe mode)}}
  do_stuff_with_ref(*foo_array[2]); // expected-remark {{not setting bounds for reference to 'Foo' (non-final class and using sub-object-safe mode)}}
}

struct Foo_final final {
  int x;
  int y;
};

void do_stuff_with_ref(Foo_final&);

void test_final_class_array() {
  Foo_final foo_array[2] = { {1, 2}, {2, 3} };
  do_stuff_with_ref(foo_array[0]); // expected-remark {{setting sub-object bounds for reference to 'Foo_final' to 8 bytes}}
  do_stuff_with_ref(foo_array[1]); // expected-remark {{setting sub-object bounds for reference to 'Foo_final' to 8 bytes}}
  do_stuff_with_ref(foo_array[2]); // expected-remark {{setting sub-object bounds for reference to 'Foo_final' to 8 bytes}}
}

// finally check the dumped statistics:
// CHECK: {
// CHECK: "cheri-bounds.NumReferencesCheckedForBoundsTightening": 12,
// CHECK: "cheri-bounds.NumTightBoundsSetOnReferences": 9,
// CHECK: }
