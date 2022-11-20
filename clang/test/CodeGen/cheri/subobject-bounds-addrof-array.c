// Check that we can set bounds on addrof expressions
// REQUIRES: asserts
// RUN: rm -f %t-s.dbg %t-a.dbg %t-v.dbg

// RUN: %cheri_purecap_cc1 -cheri-bounds=subobject-safe -O0 -std=c11 -emit-llvm %s -o %t.ll \
// RUN:   -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds \
// RUN:   -Rcheri-subobject-bounds -verify=subobject-safe,common,aggressive-or-less 2>%t-s.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,SUBOBJECT-SAFE
// RUN: FileCheck %s -input-file %t-s.dbg -check-prefixes DBG,DBG-SUBOBJECT-SAFE

// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O0 -std=c11 -emit-llvm %s -o %t.ll \
// RUN:   -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds \
// RUN:   -Rcheri-subobject-bounds -verify=aggressive,common,aggressive-or-less 2>%t-a.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,AGGRESSIVE
// RUN: FileCheck %s -input-file %t-a.dbg -check-prefixes DBG,DBG-AGGRESSIVE

// RUN: %cheri_purecap_cc1 -cheri-bounds=very-aggressive -O0 -std=c11 -emit-llvm %s -o %t.ll \
// RUN:   -mllvm -debug-only=cheri-bounds -mllvm -stats -Wno-array-bounds \
// RUN:   -Rcheri-subobject-bounds -verify=very-aggressive,common 2>%t-v.dbg
// RUN: FileCheck %s -input-file %t.ll  -check-prefixes CHECK,VERY-AGGRESSIVE
// RUN: FileCheck %s -input-file %t-v.dbg -check-prefixes DBG,DBG-VERY-AGGRESSIVE

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
  // CHECK-LABEL: define dso_local void @test_len2_array(
  // In safe mode there should not be any bounds here:
  do_stuff_untyped(&s->values);
  // common-remark@-1 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo[3]') to 12 bytes}}
  // DBG: Found constant size array type -> setting bounds for 'struct Foo[3]' address to 12
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}} i64 12)

  marker1(); // for bounding the filecheck tests
  // CHECK-LABEL: call void @marker1()

  // should only set bounds in very agressive mode here
  // aggressive mode only sets bounds for not-start and not-end of array
  // NOTE: very aggressive even sets bounds on &array[0]/&array[last_index]?

  do_stuff_untyped(&s->values[0]); // &array[0] probably means the full array
  // aggressive-or-less-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[0]}}
  // aggressive-or-less-remark@-2 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
  // very-aggressive-remark@-3 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // common-remark@-4 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

  // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

  // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[0] -> using container size -> setting bounds for 'struct Foo' address to 12
  // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[0] -> using container size -> setting bounds for 'struct Foo' address to 12

  do_stuff_untyped(&s->values[1]); // probably fine to set bounds here in aggressive (but not in safe mode)
  // subobject-safe-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[<CONSTANT>]}}
  // subobject-safe-remark@-2 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
  // aggressive-remark@-3 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // very-aggressive-remark@-4 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // common-remark@-5 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

  // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

  // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> const array index is not end and bounds==aggressive -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[<CONSTANT>] -> using container size -> setting bounds for 'struct Foo' address to 12

  do_stuff_untyped(&s->values[2]); // should only set bounds in very-agressive mode here
  // subobject-safe-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[<CONSTANT>]}}
  // aggressive-remark@-2 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on bounds on &array[<last index>]}}
  // aggressive-or-less-remark@-3 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
  // very-aggressive-remark@-4 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // common-remark@-5 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

  // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

  // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> bounds on &array[<last index>] -> using container size -> setting bounds for 'struct Foo' address to 12
  // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[<CONSTANT>] -> using container size -> setting bounds for 'struct Foo' address to 12

  do_stuff_untyped(&s->values[3]); // should only set bounds in very-agressive mode here
  // subobject-safe-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[<CONSTANT>]}}
  // aggressive-remark@-2 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on bounds on &array[<last index>]}}
  // aggressive-or-less-remark@-3 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
  // very-aggressive-remark@-4 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // common-remark@-5 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

  // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

  // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> bounds on &array[<last index>] -> using container size -> setting bounds for 'struct Foo' address to 12
  // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[<CONSTANT>] -> using container size -> setting bounds for 'struct Foo' address to 12

  do_stuff_untyped(&s->values[4]); // should only set bounds in very-agressive mode here
  // subobject-safe-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[<CONSTANT>]}}
  // aggressive-remark@-2 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on bounds on &array[<last index>]}}
  // aggressive-or-less-remark@-3 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
  // very-aggressive-remark@-4 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
  // common-remark@-5 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

  // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

  // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
  // DBG-VERY-AGGRESSIVE-NEXT: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
  // DBG-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> bounds on &array[<last index>] -> using container size -> setting bounds for 'struct Foo' address to 12
  // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> index is a constant -> &array[<CONSTANT>] -> using container size -> setting bounds for 'struct Foo' address to 12

  marker2(); // for bounding the filecheck tests
  // CHECK-LABEL: call void @marker2()

  for (int i = 0; i < 3; i++) {
    // probably okay to set bounds here since this generally means only the given object
    // TODO: would be nice to set bounds here even in safe mode but it seems like this breaks lots of C code
    do_stuff_untyped(&s->values[i]); // should set bounds here since this will usually be a single object
    // subobject-safe-remark@-1 {{using size of containing type 'struct Foo[3]' instead of object type 'struct Foo' for subobject bounds on &array[n]}}
    // subobject-safe-remark@-2 {{setting sub-object bounds for pointer to 'struct Foo' to 12 bytes}}
    // aggressive-remark@-3 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
    // very-aggressive-remark@-4 {{setting sub-object bounds for field 'values' (pointer to 'struct Foo') to 4 bytes}}
    // common-remark@-5 {{setting sub-object bounds for field 'values' (array subscript on 'struct Foo[3]') to 12 bytes}}

    // SUBOBJECT-SAFE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 12)
    // AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)
    // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)

    // DBG-NEXT: subscript 'struct Foo[3]' subobj bounds check: got MemberExpr -> subscript on constant size array -> setting bounds for 'struct Foo[3]' subscript to 12
    // DBG-SUBOBJECT-SAFE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> Index is not a constant (probably in a per-element loop) -> &array[n] -> using container size -> setting bounds for 'struct Foo' address to 12
    // DBG-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> Index is not a constant (probably in a per-element loop) -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
    // DBG-VERY-AGGRESSIVE-NEXT: subobj bounds check: got MemberExpr ->  Found array subscript -> Index is not a constant (probably in a per-element loop) -> Found record type 'struct Foo' -> compiling C and no flexible array -> setting bounds for 'struct Foo' address to 4
    }
  // CHECK: ret void
}

void test2(int *array) {
  // CHECK-LABEL: @test2(
  // Should use the full array here except in very-aggressive mode.
  // However the size of the array is not known so we should not be setting any bounds!
  do_stuff_untyped(&array[0]);
  // aggressive-or-less-remark@-1{{not setting bounds for pointer to 'int' (should set bounds on full array but size is not known)}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'int' to 4 bytes}}
  // common-remark@-3{{not setting bounds for array subscript on 'int *' (array subscript on non-array type)}}

  // SUBOBJECT-SAFE-NOT: @llvm.cheri.cap.bounds.set.i64
  // AGGRESSIVE-SAFE-NOT: @llvm.cheri.cap.bounds.set.i64
  // VERY-AGGRESSIVE: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %{{.+}}, i64 4)
  // CHECK: ret void

  // DBG-NEXT: subscript 'int * __capability' subobj bounds check: array subscript on non-array type -> not setting bounds
  // DBG-SUBOBJECT-SAFE-NEXT: address 'int' subobj bounds check: Found array subscript -> index is a constant -> should set bounds on full array but size is not known -> not setting bounds
  // DBG-AGGRESSIVE-NEXT: address 'int' subobj bounds check: Found array subscript -> index is a constant -> should set bounds on full array but size is not known -> not setting bounds
  // DBG-VERY-AGGRESSIVE-NEXT: address 'int' subobj bounds check: Found array subscript -> index is a constant -> bounds-mode is very-aggressive -> bounds on array[CONST] are fine -> Found scalar type -> setting bounds for 'int' address to 4
}

#ifdef NOTYET
// FIXME: should handle this case correctly:
void test_multidim_array(struct with_2d_array* s, int index) {
  int array2d[3][4];
  do_stuff(&(array2d)[0][0]));
  do_stuff(&(array2d)[index][index]));
}
#endif

// DBG-LABEL: ... Statistics Collected ...
// DBG-NOT: cheri-bounds
// DBG: 8 cheri-bounds     - Number of & operators checked for tightening bounds
// DBG-NOT: cheri-bounds
// DBG: 7 cheri-bounds     - Number of [] operators checked for tightening bounds
// DBG-NOT: cheri-bounds
// DBG-VERY-AGGRESSIVE-NEXT: 8 cheri-bounds     - Number of & operators where bounds were tightened

// DBG-AGGRESSIVE-NEXT:      4 cheri-bounds     - Number of & operators where container bounds were used
// DBG-AGGRESSIVE-NEXT:      3 cheri-bounds     - Number of & operators where bounds were tightened

// DBG-SUBOBJECT-SAFE-NEXT:  6 cheri-bounds     - Number of & operators where container bounds were used
// DBG-SUBOBJECT-SAFE-NEXT:  1 cheri-bounds     - Number of & operators where bounds were tightened
// DBG-NOT: cheri-bounds
// DBG: 6 cheri-bounds - Number of [] operators where bounds were tightened
// DBG-NOT: cheri-bounds
