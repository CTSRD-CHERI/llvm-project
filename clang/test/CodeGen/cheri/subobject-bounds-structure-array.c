// RUN: rm -f %t.dbg
// RUN: %cheri_purecap_cc1 -cheri-bounds=everywhere-unsafe -O2 -emit-llvm %s -o - \
// RUN:   -Wcheri-subobject-bounds -Rcheri-subobject-bounds -mllvm -debug-only="cheri-bounds" -verify 2>%t.dbg
// RUN: cat %t.dbg
// RUN: false
// REQUIRES: asserts
void use_buf(void *);

typedef struct {
  long x;
  int buf[10];
  char c;
} struct_with_array;

int test_struct_with_array1(struct_with_array *s, int index) {
  // should set bounds to 10
  return s->buf[index]; // expected-remark{{setting sub-object bounds for field 'buf' (array subscript on 'int [10]') to 40 bytes}}
}

struct_with_array test_struct_with_array2(struct_with_array *s, int index) {
  // can't set bounds here, have to trust the caller's bounds
  return s[index]; // expected-remark{{not setting bounds for array subscript on 'struct_with_array * __capability' (array subscript on non-array type)}}
}

typedef struct {
  long x;
  int *buf;
  char c;
} struct_with_ptr;

int test_struct_with_ptr1(struct_with_ptr *s, int index) {
  return s->buf[index]; // expected-remark{{not setting bounds for array subscript on 'int * __capability' (array subscript on non-array type)}}
}

struct_with_ptr test_struct_with_ptr2(struct_with_ptr *s, int index) {
  return s[index]; // expected-remark{{not setting bounds for array subscript on 'struct_with_ptr * __capability' (array subscript on non-array type)}}
}

typedef struct {
  int x;
  char buf[];
} struct_vla;
int test_vla_a(struct_vla *s, int index) {
  // can't set bounds here, have to trust the caller's bounds
  s->buf[index] = 'A'; // expected-remark{{not setting bounds for array subscript on 'char []' (incomplete type)}}
  return 0;
}
struct_vla test_vla_b(int index) {
  struct_vla s2;
  s2.buf[index] = 'A'; // expected-remark{{not setting bounds for array subscript on 'char []' (incomplete type)}}
  return s2;           // prevent s2 from being optimzed out
}
