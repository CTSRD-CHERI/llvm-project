// Check that we can set bounds on addrof expressions
// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c11 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds -mllvm -stats 2>%t.dbg | %cheri_FileCheck %s -check-prefixes CHECK,AGGRESSIVE
// RUN: %cheri_FileCheck -input-file %t.dbg %s -check-prefix DBG
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -DUSE_BUITLTIN_ADDROF -O2 -std=c11 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds -mllvm -stats 2>%t.dbg | %cheri_FileCheck %s -check-prefixes CHECK,AGGRESSIVE
// RUN: %cheri_FileCheck -input-file %t.dbg %s -check-prefix DBG

#ifdef USE_BUITLTIN_ADDROF
#define TAKE_ADDRESS(obj) __builtin_addressof(obj)
#else
#define TAKE_ADDRESS(obj) &(obj)
#endif

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
  do_stuff_with_int(TAKE_ADDRESS(s->n.a));
  // DBG: Found scalar type -> setting bounds for 'int' addrof to 4
  // CHECK: [[BOUNDED_INT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %addrof.with.bounds = bitcast i8 addrspace(200)* [[BOUNDED_INT]] to i32 addrspace(200)*
  do_stuff_with_nested(TAKE_ADDRESS(s->n));
  // DBG-NEXT: Found record type 'struct Nested' -> compiling C and no flexible array -> setting bounds for 'struct Nested' addrof to 12
  // CHECK: [[BOUNDED_STRUCT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 12)
  // CHECK-NEXT: %addrof.with.bounds2 = bitcast i8 addrspace(200)* [[BOUNDED_STRUCT]] to %struct.Nested addrspace(200)*
  do_stuff_with_float(TAKE_ADDRESS(s->f1));
  // DBG-NEXT: Found scalar type -> setting bounds for 'float' addrof to 4
  // CHECK: [[BOUNDED_FLOAT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %addrof.with.bounds3 = bitcast i8 addrspace(200)* [[BOUNDED_FLOAT]] to float addrspace(200)*
}

typedef void (*fn_ptr_ty)(void);

struct ContainsFnPtr {
  fn_ptr_ty fn_ptr;
};

void do_stuff_with_fn_ptr(fn_ptr_ty);
void do_stuff_with_void_ptr(void*);
void do_stuff_with_fn_ptr_ptr(fn_ptr_ty*);
void foo(void);


void test_fnptr(struct ContainsFnPtr *s) {
  do_stuff_with_fn_ptr(&foo);
  // DBG-NEXT: subobj bounds check: cannot set bounds on function addressof/reference
  do_stuff_with_fn_ptr_ptr(&s->fn_ptr);
  // DBG-NEXT: subobj bounds check: got MemberExpr -> Found scalar type -> setting bounds for 'fn_ptr_ty' addrof to [[$CAP_SIZE]]
  fn_ptr_ty fnptr_array[4];
  struct ContainsFnPtr onstack;
  do_stuff_with_fn_ptr_ptr(&onstack.fn_ptr);
  // DBG-NEXT: subobj bounds check: got MemberExpr -> Found scalar type -> setting bounds for 'fn_ptr_ty' addrof to [[$CAP_SIZE]]
  do_stuff_with_fn_ptr_ptr(&fnptr_array[2]);
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> const array index is not end and bounds==aggressive -> Found scalar type -> setting bounds for 'fn_ptr_ty' addrof to [[$CAP_SIZE]]
}


typedef _Atomic(int) atomic_int_t;
extern atomic_int_t at_int;
struct WithAtomicInt {
  atomic_int_t value;
};

void test_atomic(atomic_int_t* array, struct WithAtomicInt *s) {
  do_stuff_with_void_ptr(&array[0]);
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> const array index is 0 or end of array
  do_stuff_with_void_ptr(&array[2]);
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> const array index is not end and bounds==aggressive -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' addrof to 4
  do_stuff_with_void_ptr(&at_int);
  // DBG-NEXT: subobj bounds check: unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' addrof to 4
  struct WithAtomicInt onstack;
  do_stuff_with_void_ptr(&onstack.value);
  // DBG-NEXT: subobj bounds check: got MemberExpr -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' addrof to 4
  do_stuff_with_void_ptr(&s->value);
  // DBG-NEXT: subobj bounds check: got MemberExpr -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' addrof to 4

}

// DBG-LABEL: ... Statistics Collected ...
// DBG: 12 cheri-bounds     - Number of & operators checked for tightening bounds
// DBG: 10 cheri-bounds     - Number of & operators where bounds were tightend

