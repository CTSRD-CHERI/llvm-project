// Check that we can set bounds on addrof expressions
// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -cheri-bounds=subobject-safe -O0 -std=c11 -emit-llvm %s -o %t.ll -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds 2>%t.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,SUBOBJECT-SAFE
// RUN: FileCheck %s -input-file %t.dbg -check-prefixes DBG,DBG-SUBOBJECT-SAFE
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O0 -std=c11 -emit-llvm %s -o %t.ll -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds 2>%t.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,AGGRESSIVE
// RUN: FileCheck %s -input-file %t.dbg -check-prefixes DBG,DBG-AGGRESSIVE
// RUN: %cheri_purecap_cc1 -cheri-bounds=very-aggressive -O0 -std=c11 -emit-llvm %s -o %t.ll -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds 2>%t.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,VERY-AGGRESSIVE
// RUN: FileCheck %s -input-file %t.dbg -check-prefixes DBG,DBG-VERY-AGGRESSIVE

struct WithTrailingArray {
  int len;
  char buf[];
};

struct WithLen1ArrayAtEnd {
  int len;
  char buf[1];
};

struct WithLen0ArrayAtEnd {
  int len;
  char buf[0];
};

struct WithLen1ArrayNotAtEnd {
  int len;
  char buf[1];
  int something_else;
};

struct Foo {
  int value;
};

struct WithLen2Array {
  int len;
  struct Foo values[3];
};

void do_stuff_untyped(void *);
void marker1(void);
void marker2(void);
void marker3(void);

void test_len2_array(struct WithLen2Array *s) {
  // CHECK-LABEL: define void @test_len2_array(
  // In safe mode there should not be any bounds here:
  do_stuff_untyped(&s->values); // should set bounds here to the full array (even in safe mode)
  // DBG: Found constant size array type -> setting bounds for 'struct Foo [3]' addrof to 12
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}} i64 12)

  marker1(); // for bounding the filecheck tests
  // CHECK-LABEL: call void @marker1()

  // no bounds in safe mode for any constant indices
  // SUBOBJECT-SAFE-NOT: llvm.cheri.cap.bounds.set

  // should only set bounds in very agressive mode here
  // aggressive mode only sets bounds for not-start and not-end of array
  // NOTE: very aggressive even sets bounds on &array[0]/&array[last_index]?
  // AGGRESSIVE-NOT: llvm.cheri.cap.bounds.set

  do_stuff_untyped(&s->values[0]); // should only set bounds in very-agressive mode here
  // AGGRESSIVE-NOT: llvm.cheri.cap.bounds.set
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> setting bounds for 'struct Foo' addrof to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> const array index is 0 or end of array
  // DBG-SUBOBJECT-SAFE-NEXT: Found array subscript -> Index is a constant -> bounds on array[CONST] only with mode>=aggressive


  do_stuff_untyped(&s->values[1]); // probably fine to set bounds here in aggressive (but not in safe mode)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> setting bounds for 'struct Foo' addrof to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> const array index is not end and bounds==aggressive -> setting bounds for 'struct Foo' addrof to 4
  // DBG-SUBOBJECT-SAFE-NEXT: Found array subscript -> Index is a constant -> bounds on array[CONST] only with mode>=aggressive

  do_stuff_untyped(&s->values[2]); // should only set bounds in very-agressive mode here
  // AGGRESSIVE-NOT: llvm.cheri.cap.bounds.set
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> setting bounds for 'struct Foo' addrof to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> const array index is 0 or end of array
  // DBG-SUBOBJECT-SAFE-NEXT: Found array subscript -> Index is a constant -> bounds on array[CONST] only with mode>=aggressive
  do_stuff_untyped(&s->values[3]); // should only set bounds in very-agressive mode here
  // AGGRESSIVE-NOT: llvm.cheri.cap.bounds.set
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> setting bounds for 'struct Foo' addrof to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> const array index is 0 or end of array
  // DBG-SUBOBJECT-SAFE-NEXT: Found array subscript -> Index is a constant -> bounds on array[CONST] only with mode>=aggressive
  do_stuff_untyped(&s->values[4]); // should only set bounds in very-agressive mode here
  // AGGRESSIVE-NOT: llvm.cheri.cap.bounds.set
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> setting bounds for 'struct Foo' addrof to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> Index is a constant -> const array index is 0 or end of array
  // DBG-SUBOBJECT-SAFE-NEXT: Found array subscript -> Index is a constant -> bounds on array[CONST] only with mode>=aggressive

  marker2(); // for bounding the filecheck tests
  // CHECK-LABEL: call void @marker2()

  for (int i = 0; i < 3; i++) {
    // probably okay to set bounds here since this generally means only the given object
    // Even in safe mode we set bounds here
    do_stuff_untyped(&s->values[i]); // should set bounds here since this will usually be a single object
    // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %{{.+}}, i64 4)
    // DBG-NEXT: Found array subscript -> Index is not a constant (probably in a per-element loop) -> setting bounds for 'struct Foo' addrof to 4
  }
  // CHECK: ret void
}

// DBG-LABEL: ... Statistics Collected ...
// DBG:                 7 cheri-bounds     - Number of & operators checked for tightening bounds
// DBG-VERY-AGGRESSIVE-NEXT: 7 cheri-bounds     - Number of & operators where bounds were tightend
// DBG-AGGRESSIVE-NEXT:      3 cheri-bounds     - Number of & operators where bounds were tightend
// DBG-SUBOBJECT-SAFE-NEXT:  2 cheri-bounds     - Number of & operators where bounds were tightend
