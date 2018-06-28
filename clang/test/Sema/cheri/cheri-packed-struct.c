// RUN: %cheri_cc1 -fsyntax-only -verify %s -target-abi purecap
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

struct correct_when_used_in_array_but_bad_alignof {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
} __attribute__((packed));

struct bad_uintptr_array_1 { // expected-note-re {{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed));

struct bad_uintptr_array_2 { // expected-note-re {{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}} expected-warning-re{{alignment (8) of 'struct bad_uintptr_array_2' is less than the required capability alignment ({{16|32}})}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));

struct good_uintptr_array {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));

// same again but the member is a struct containing a capability
struct bad_cap_struct_array_1 { // expected-note-re{{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed));

struct bad_cap_struct_array_2 { // expected-note-re{{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}} expected-warning-re{{alignment (8) of 'struct bad_cap_struct_array_2' is less than the required capability alignment ({{16|32}})}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));

struct good_cap_struct_array {
  char before[sizeof(void*)];
  struct cap_struct cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));

#ifdef NOTYET
// We don't look at the offset the the capability inside the contained struct yet
// Check that we get a warning also with capabilities at odd offsets in structs
struct good_packed_cap_struct_array {
  // 28 bytes alignment to ensure the packed struct is aligned correctly
  int pad[7];
  struct packed_cap_struct cap;
} __attribute__((packed)) ;
struct bad_packed_cap_struct_array {
  // 28 bytes alignment to ensure the packed struct is aligned correctly
  int pad[7];
  struct packed_cap_struct cap;  // expected-warning {{Capability field at offset 32 in packed structure will trap if structure is used in an array}}
  int bad;
} __attribute__((packed));
#endif

int main() {
  struct padded_cap a;
  struct packed_cap b;
  struct padded_uintptr_t c;
  struct packed_uintptr_t d;
  return sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);
}
