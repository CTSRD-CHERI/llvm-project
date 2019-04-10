// Check that we only set bounds on variable length array member expressions in very-aggressive mode
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c11 -emit-llvm -xc %s -o /dev/null \
// RUN:   -Wcheri-subobject-bounds -Rcheri-subobject-bounds -verify
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c++11 -emit-llvm -xc++ %s -o /dev/null \
// RUN:   -Wcheri-subobject-bounds -Rcheri-subobject-bounds -verify

struct WithVLA {
  float x;
  struct CharVLA {
    int size;
    char data[];
  } buf;
};

struct NestedVLA {
  double a;
  struct WithVLA vla_struct;
};

struct WithIntVLA {
  long x;
  struct IntVLA {
    int size;
    int data[];
  } buf;
};

void do_stuff_untyped(void *);

void test_subobject_addrof_vla(struct WithVLA *s1, struct WithIntVLA *s2, struct NestedVLA *n) {
  do_stuff_untyped(&s1->buf);      // expected-remark{{not setting bounds for pointer to 'struct CharVLA' (has flexible array member)}}
  do_stuff_untyped(&s1->buf.size); // expected-remark{{setting sub-object bounds for field 'size' (pointer to 'int') to 4 bytes}}
  // TODO: I guess this should be a slightly different remark message (VLA instead of incomplete type)
  do_stuff_untyped(&s1->buf.data); // expected-remark{{not setting bounds for pointer to 'char []' (incomplete type)}}
  do_stuff_untyped(&s2->buf);      // expected-remark{{not setting bounds for pointer to 'struct IntVLA' (has flexible array member)}}
  do_stuff_untyped(&s2->buf.size); // expected-remark{{setting sub-object bounds for field 'size' (pointer to 'int') to 4 bytes}}
  do_stuff_untyped(&s2->buf.data); // expected-remark{{not setting bounds for pointer to 'int []' (incomplete type)}}
  do_stuff_untyped(&n->vla_struct);   // expected-remark{{not setting bounds for pointer to 'struct WithVLA' (has flexible array member)}}
  do_stuff_untyped(&n->vla_struct.x); // expected-remark{{setting sub-object bounds for field 'x' (pointer to 'float') to 4 bytes}}
  do_stuff_untyped(&n->vla_struct.buf);      // expected-remark{{not setting bounds for pointer to 'struct CharVLA' (has flexible array member)}}
  do_stuff_untyped(&n->vla_struct.buf.size); // expected-remark{{setting sub-object bounds for field 'size' (pointer to 'int') to 4 bytes}}
  do_stuff_untyped(&n->vla_struct.buf.data); // expected-remark{{not setting bounds for pointer to 'char []' (incomplete type)}}
}

struct VLAContainerNoLength {
  int x;
  struct ArbitraryLengthCharArray {
    int size;
    char data[];
  } buf;
} global_vn;

struct VLAContainerLen0 {
  int x;
  struct ZeroLengthCharArray {
    int size;
    char data[0];
  } buf;
} global_v0;

struct VLAContainerLen1 {
  int x;
  struct LengthOneCharArray {
    int size;
    char data[1];
  } buf;
} global_v1;

struct NotVLAContainerLen2 {
  int x;
  struct LengthTwoCharArray {
    int size;
    char data[2];
  } buf;
} global_v2;

void test2(void* ptr) {
  struct VLAContainerNoLength* vn = (struct VLAContainerNoLength*)ptr;
  do_stuff_untyped(&global_vn);    // expected-remark{{not setting bounds for pointer to 'struct VLAContainerNoLength' (has flexible array member)}}
  do_stuff_untyped(&vn->buf);      // expected-remark{{not setting bounds for pointer to 'struct ArbitraryLengthCharArray' (has flexible array member)}}
  do_stuff_untyped(&vn->buf.data); // expected-remark{{not setting bounds for pointer to 'char []' (incomplete type)}}
  struct VLAContainerLen0* v0 = (struct VLAContainerLen0*)ptr;
  do_stuff_untyped(&global_v0);    // expected-remark{{not setting bounds for pointer to 'struct VLAContainerLen0' (has flexible array member)}}
  do_stuff_untyped(&v0->buf);      // expected-remark{{not setting bounds for pointer to 'struct ZeroLengthCharArray' (has flexible array member)}}
  do_stuff_untyped(&v0->buf.data); // expected-remark{{not setting bounds for pointer to 'char [0]' (member is potential variable length array)}}
  struct VLAContainerLen1* v1 = (struct VLAContainerLen1*)ptr;
  do_stuff_untyped(&global_v1);    // expected-remark{{not setting bounds for pointer to 'struct VLAContainerLen1' (has flexible array member)}}
  do_stuff_untyped(&v1->buf);      // expected-remark{{not setting bounds for pointer to 'struct LengthOneCharArray' (has flexible array member)}}
  do_stuff_untyped(&v1->buf.data); // expected-remark{{not setting bounds for pointer to 'char [1]' (member is potential variable length array)}}
  struct NotVLAContainerLen2* v2 = (struct NotVLAContainerLen2*)ptr;
  do_stuff_untyped(&global_v2); // expected-remark{{setting bounds for pointer to 'struct NotVLAContainerLen2' to 12 bytes}}
  do_stuff_untyped(&v2->buf); // expected-remark{{sub-object bounds for field 'buf' (pointer to 'struct LengthTwoCharArray') to 8 bytes}}
  do_stuff_untyped(&v2->buf.data); // expected-remark{{setting sub-object bounds for field 'data' (pointer to 'char [2]') to 2 bytes}}
}

int test_local_vla(int len, int index) {
  int buf[len];
  do_stuff_untyped(&buf); // expected-remark{{not setting bounds for pointer to 'int [len]' (variable length array type)}}
  return buf[index];      // expected-remark{{not setting bounds for array subscript on 'int [len]' (array subscript on variable size type)}}
  // expected-remark@-1{{not setting bounds for array decay on 'int [len]' (array decay on variable size type)}}
}

#ifdef __cplusplus
template<int SIZE>
struct MaybeVLA {
  int size;
  char data[SIZE];
};

template<typename T> void do_stuff_reference(T&);

void test_maybe_vla(void) {
  MaybeVLA<0> mvla0;
  do_stuff_reference(mvla0);      // expected-remark{{not setting bounds for reference to 'MaybeVLA<0>' (has flexible array member)}}
  do_stuff_untyped(&mvla0);       // expected-remark{{not setting bounds for pointer to 'MaybeVLA<0>' (has flexible array member)}}
  do_stuff_reference(mvla0.data); // expected-remark{{not setting bounds for reference to 'char [0]' (member is potential variable length array)}}
  do_stuff_untyped(&mvla0.data);  // expected-remark{{not setting bounds for pointer to 'char [0]' (member is potential variable length array)}}

  MaybeVLA<1> mvla1;
  do_stuff_reference(mvla1);      // expected-remark{{not setting bounds for reference to 'MaybeVLA<1>' (has flexible array member)}}
  do_stuff_untyped(&mvla1);       // expected-remark{{not setting bounds for pointer to 'MaybeVLA<1>' (has flexible array member)}}
  do_stuff_reference(mvla1.data); // expected-remark{{not setting bounds for reference to 'char [1]' (member is potential variable length array)}}
  do_stuff_untyped(&mvla1.data);  // expected-remark{{not setting bounds for pointer to 'char [1]' (member is potential variable length array)}}

  MaybeVLA<2> mvla2;
  do_stuff_reference(mvla2); // expected-remark{{setting bounds for reference to 'MaybeVLA<2>' to 8 bytes}}
  do_stuff_untyped(&mvla2); // expected-remark{{setting bounds for pointer to 'MaybeVLA<2>' to 8 bytes}}
  do_stuff_reference(mvla2.data); //expected-remark{{setting sub-object bounds for field 'data' (reference to 'char [2]') to 2 bytes}}
  do_stuff_untyped(&mvla2.data);  //expected-remark{{setting sub-object bounds for field 'data' (pointer to 'char [2]') to 2 bytes}}
}
#endif
