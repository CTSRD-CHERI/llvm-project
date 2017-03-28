// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi purecap
// Ensure that we warn on structs packed structs with misaligned capabilities

struct cap_struct {
  void* __capability cap;
  int x;
};


struct padded_cap {
  int i;
  void* __capability cap;
};
struct packed_cap {
  int i;
  void* __capability cap;  // expected-warning {{Under aligned capability field at offset 4 in packed structure will trap if accessed}}
} __attribute__((packed));


struct padded_cap_struct {
  int i;
  struct cap_struct cap;
};

struct packed_cap_struct {
  int i;
  struct cap_struct cap;  // expected-warning {{Under aligned structure field at offset 4 in packed structure containing capabilities will trap if capability field is accessed}}
} __attribute__((packed));


struct padded_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;
};
struct packed_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;  // expected-warning {{Under aligned capability field at offset 4 in packed structure will trap if accessed}}
} __attribute__((packed));



// check that we warn for structs that are wrong when used in arrays

struct correctly_aligned_but_size_not_multiple {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
};
struct bad_uintptr_array_1 { // expected-note{{Add __attribute__((aligned(32))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning {{Capability field at offset 32 in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed));

struct bad_uintptr_array_2 { // expected-note{{Add __attribute__((aligned(32))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning {{Capability field at offset 32 in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));

struct good_uintptr_array {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));

// same again but the member is a struct containing a capability
struct bad_cap_struct_array_1 { // expected-note{{Add __attribute__((aligned(32))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning {{Capability field at offset 32 in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed));

struct bad_cap_struct_array_2 { // expected-note{{Add __attribute__((aligned(32))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning {{Capability field at offset 32 in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));

struct good_cap_struct_array {
  char before[sizeof(void*)];
  struct cap_struct cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));


int main() {
  struct padded_cap a;
  struct packed_cap b;
  struct padded_uintptr_t c;
  struct packed_uintptr_t d;
  return sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);
}
