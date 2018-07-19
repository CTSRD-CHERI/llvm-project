// Check that we can set bounds on addrof expressions
// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c11 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds -mllvm -stats 2>%t.dbg | FileCheck %s -check-prefixes CHECK,AGGRESSIVE
// RUN: FileCheck -input-file %t.dbg %s -check-prefix DBG

struct Nested {
    int a;
    int b;
    int c;
};

struct WithNested {
  float f1;
  struct Nested n;
  float f2;
};

void do_stuff_with_int(int*);
void do_stuff_with_float(float*);
void do_stuff_with_nested(struct Nested* nptr);

void test_subobject_addrof_basic(struct WithNested* s) {
  // CHECK-LABEL: @test_subobject_addrof_basic(
  do_stuff_with_int(&s->n.a);
  // DBG: Found scalar type -> setting bounds for 'int' addrof to 4
  // CHECK: [[BOUNDED_INT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %addrof.with.bounds = bitcast i8 addrspace(200)* [[BOUNDED_INT]] to i32 addrspace(200)*
  do_stuff_with_nested(&s->n);
  // DBG-NEXT: Found record type 'struct Nested' -> compiling C and no flexible array -> setting bounds for 'struct Nested' addrof to 12
  // CHECK: [[BOUNDED_STRUCT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 12)
  // CHECK-NEXT: %addrof.with.bounds2 = bitcast i8 addrspace(200)* [[BOUNDED_STRUCT]] to %struct.Nested addrspace(200)*
  do_stuff_with_float(&s->f1);
  // DBG-NEXT: Found scalar type -> setting bounds for 'float' addrof to 4
  // CHECK: [[BOUNDED_FLOAT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %addrof.with.bounds3 = bitcast i8 addrspace(200)* [[BOUNDED_FLOAT]] to float addrspace(200)*
}

void do_stuff_untyped(void*);

// DBG-LABEL: ... Statistics Collected ...
// DBG: 3 cheri-bounds     - Number of & operators checked for tightening bounds
// DBG: 3 cheri-bounds     - Number of & operators where bounds were tightend

