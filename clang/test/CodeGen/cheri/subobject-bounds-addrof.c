// Check that we can set bounds on addrof expressions
// REQUIRES: asserts
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c11 -emit-llvm %s -o %t.ll -Wcheri-subobject-bounds -Rcheri-subobject-bounds -verify
// RUN: %cheri_FileCheck %s -check-prefixes CHECK,AGGRESSIVE -input-file=%t.ll
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -DUSE_BUITLTIN_ADDROF -O2 -std=c11 -emit-llvm %s -o - -mllvm -debug-only=cheri-bounds -mllvm -stats 2>%t.dbg | %cheri_FileCheck %s -check-prefixes CHECK,AGGRESSIVE
// RUN: %cheri_FileCheck -input-file %t.dbg %s -check-prefix DBG -implicit-check-not cheri-bounds

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
  do_stuff_with_int(TAKE_ADDRESS(s->n.a)); // expected-remark {{setting sub-object bounds for field 'a' (pointer to 'int') to 4 bytes}}
  // DBG: Found scalar type -> setting bounds for 'int' address to 4
  // CHECK: [[BOUNDED_INT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %address.with.bounds = bitcast i8 addrspace(200)* [[BOUNDED_INT]] to i32 addrspace(200)*
  do_stuff_with_nested(TAKE_ADDRESS(s->n)); // expected-remark {{setting sub-object bounds for field 'n' (pointer to 'struct Nested') to 12 bytes}}
  // DBG-NEXT: Found record type 'struct Nested' -> compiling C and no flexible array -> setting bounds for 'struct Nested' address to 12
  // CHECK: [[BOUNDED_STRUCT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 12)
  // CHECK-NEXT: %address.with.bounds2 = bitcast i8 addrspace(200)* [[BOUNDED_STRUCT]] to %struct.Nested addrspace(200)*
  do_stuff_with_float(TAKE_ADDRESS(s->f1)); // expected-remark {{setting sub-object bounds for field 'f1' (pointer to 'float') to 4 bytes}}
  // DBG-NEXT: Found scalar type -> setting bounds for 'float' address to 4
  // CHECK: [[BOUNDED_FLOAT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 4)
  // CHECK-NEXT: %address.with.bounds3 = bitcast i8 addrspace(200)* [[BOUNDED_FLOAT]] to float addrspace(200)*
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
  do_stuff_with_fn_ptr(&foo); // expected-remark {{not setting bounds for pointer to 'void (void)' (address of function)}}
  // DBG-NEXT: subobj bounds check: address of function -> not setting bounds
  do_stuff_with_fn_ptr_ptr(&s->fn_ptr); // expected-remark-re {{setting sub-object bounds for field 'fn_ptr' (pointer to 'fn_ptr_ty' (aka 'void (* __capability)(void)')) to {{16|32}} bytes}}
  // DBG-NEXT: subobj bounds check: got MemberExpr -> Found scalar type -> setting bounds for 'fn_ptr_ty' address to [[#CAP_SIZE]]
  fn_ptr_ty fnptr_array[4];
  struct ContainsFnPtr onstack;
  do_stuff_with_fn_ptr_ptr(&onstack.fn_ptr); // expected-remark-re {{setting sub-object bounds for field 'fn_ptr' (pointer to 'fn_ptr_ty' (aka 'void (* __capability)(void)')) to {{16|32}} bytes}}
  // DBG-NEXT: subobj bounds check: got MemberExpr -> Found scalar type -> setting bounds for 'fn_ptr_ty' address to [[#CAP_SIZE]]
  do_stuff_with_fn_ptr_ptr(&fnptr_array[2]); // expected-remark-re {{setting sub-object bounds for pointer to 'fn_ptr_ty' (aka 'void (* __capability)(void)') to {{16|32}} bytes}}
  //  expected-remark-re@-1{{setting bounds for array subscript on 'fn_ptr_ty [4]' to {{64|128}} bytes}}
  // DBG-NEXT: subscript 'fn_ptr_ty [4]' subobj bounds check: subscript on constant size array -> setting bounds for 'fn_ptr_ty [4]' subscript to {{64|128}}
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> const array index is not end and bounds==aggressive -> Found scalar type -> setting bounds for 'fn_ptr_ty' address to [[#CAP_SIZE]]
}


typedef _Atomic(int) atomic_int_t;
extern atomic_int_t at_int;
struct WithAtomicInt {
  atomic_int_t value;
};

void test_atomic(atomic_int_t* array, struct WithAtomicInt *s) {
  do_stuff_with_void_ptr(&array[0]); // expected-remark {{not setting bounds for pointer to 'atomic_int_t' (aka '_Atomic(int)') (should set bounds on full array but size is not known)}}
  // expected-remark@-1{{not setting bounds for array subscript on 'atomic_int_t * __capability' (aka '_Atomic(int) * __capability') (array subscript on non-array type)}}
  // DBG-NEXT: subscript 'atomic_int_t * __capability' subobj bounds check: array subscript on non-array type -> not setting bounds
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> should set bounds on full array but size is not known -> not setting bounds
  do_stuff_with_void_ptr(&array[2]); // expected-remark {{setting sub-object bounds for pointer to 'atomic_int_t' (aka '_Atomic(int)') to 4 bytes}}
  // expected-remark@-1{{not setting bounds for array subscript on 'atomic_int_t * __capability' (aka '_Atomic(int) * __capability') (array subscript on non-array type)}}
  // DBG-NEXT: subscript 'atomic_int_t * __capability' subobj bounds check: array subscript on non-array type -> not setting bounds
  // DBG-NEXT: subobj bounds check: Found array subscript -> Index is a constant -> const array index is not end and bounds==aggressive -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' address to 4
  do_stuff_with_void_ptr(&at_int); // expected-remark {{setting bounds for pointer to 'atomic_int_t' (aka '_Atomic(int)') to 4 bytes}}
  // DBG-NEXT: subobj bounds check: unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' address to 4
  struct WithAtomicInt onstack;
  do_stuff_with_void_ptr(&onstack.value); // expected-remark {{setting sub-object bounds for field 'value' (pointer to 'atomic_int_t' (aka '_Atomic(int)')) to 4 bytes}}
  // DBG-NEXT: subobj bounds check: got MemberExpr -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' address to 4
  do_stuff_with_void_ptr(&s->value); // expected-remark {{setting sub-object bounds for field 'value' (pointer to 'atomic_int_t' (aka '_Atomic(int)')) to 4 bytes}}
  // DBG-NEXT: subobj bounds check: got MemberExpr -> unwrapping _Atomic type -> Found scalar type -> setting bounds for 'atomic_int_t' address to 4
}

typedef signed char v4i8 __attribute__ ((vector_size(4)));
v4i8 global_vector = {1, 2, 3, 4};
typedef int ext_vector_size_int32_8 __attribute__((ext_vector_type(8)));

int test_vector(v4i8 v4, ext_vector_size_int32_8 v8) {
  do_stuff_with_void_ptr(&v4); // expected-remark{{setting bounds for pointer to 'v4i8' (vector of 4 'signed char' values) to 4 bytes}}
  // DBG-NEXT: address 'v4i8' subobj bounds check: Found vector type -> setting bounds for 'v4i8' address to 4
  do_stuff_with_void_ptr(&v8); // expected-remark{{setting bounds for pointer to 'ext_vector_size_int32_8' (vector of 8 'int' values) to 32 bytes}}
  // DBG-NEXT: address 'ext_vector_size_int32_8' subobj bounds check: Found vector type -> setting bounds for 'ext_vector_size_int32_8' address to 32
  return 0;
}

// DBG-NEXT: ===-------------------------------------------------------------------------===
// DBG-NEXT: ... Statistics Collected ...
// DBG-NEXT: ===-------------------------------------------------------------------------===
// DBG: 14 cheri-bounds - Number of & operators checked for tightening bounds
// DBG:  3 cheri-bounds - Number of [] operators checked for tightening bounds
// DBG: 12 cheri-bounds - Number of & operators where bounds were tightened
// DBG:  1 cheri-bounds - Number of [] operators where bounds were tightened
