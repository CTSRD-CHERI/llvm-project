// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - -verify | %cheri_FileCheck %s
// Ensure that we warn on structs packed structs with misaligned capabilities

struct cap_struct {
  void* __capability cap;
  int x;
};
struct cap_struct cap_struct_global;
// CHECK: @cap_struct_global = common addrspace(200) global %struct.cap_struct zeroinitializer, align [[$CAP_SIZE]]

struct padded_cap {
  int i;
  void* __capability cap;
};
struct padded_cap padded_cap_global;
// CHECK: @padded_cap_global = common addrspace(200) global %struct.padded_cap zeroinitializer, align [[$CAP_SIZE]]


struct packed_cap { // expected-warning{{alignment (1) of 'struct packed_cap' is less than the required capability alignment}}
  int i;
  void* __capability cap;  // expected-warning {{Under aligned capability field at offset 4 in packed structure will trap if accessed}}
} __attribute__((packed));
struct packed_cap packed_cap_global;
// CHECK: @packed_cap_global = common addrspace(200) global %struct.packed_cap zeroinitializer, align 1

struct padded_cap_struct {
  int i;
  struct cap_struct cap;
};
struct padded_cap_struct padded_cap_struct_global;
// CHECK: @padded_cap_struct_global = common addrspace(200) global %struct.padded_cap_struct zeroinitializer, align [[$CAP_SIZE]]

struct packed_cap_struct { // expected-warning{{alignment (1) of 'struct packed_cap_struct' is less than the required capability alignment}}
  int i;
  struct cap_struct cap;  // expected-warning {{Under aligned structure field at offset 4 in packed structure containing capabilities will trap if capability field is accessed}}
} __attribute__((packed));
struct packed_cap_struct packed_cap_struct_global;
// CHECK: @packed_cap_struct_global = common addrspace(200) global %struct.packed_cap_struct zeroinitializer, align 1


struct padded_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;
};
struct padded_uintptr_t padded_uintptr_t_global;
// CHECK: @padded_uintptr_t_global = common addrspace(200) global %struct.padded_uintptr_t zeroinitializer, align [[$CAP_SIZE]]

struct packed_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;  // expected-warning {{Under aligned capability field at offset 4 in packed structure will trap if accessed}}
} __attribute__((packed,annotate("underaligned_capability")));
struct packed_uintptr_t packed_uintptr_t_global;
// CHECK: @packed_uintptr_t_global = common addrspace(200) global %struct.packed_uintptr_t zeroinitializer, align 1


// check that we warn for structs that are wrong when used in arrays
struct correctly_aligned_but_size_not_multiple {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
};
struct correctly_aligned_but_size_not_multiple correctly_aligned_but_size_not_multiple_global;
// CHECK: @correctly_aligned_but_size_not_multiple_global = common addrspace(200) global %struct.correctly_aligned_but_size_not_multiple zeroinitializer, align [[$CAP_SIZE]]


struct correct_when_used_in_array_but_bad_alignof {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
} __attribute__((packed,annotate("underaligned_capability")));

struct correct_when_used_in_array_but_bad_alignof correct_when_used_in_array_but_bad_alignof_global;
// CHECK: @correct_when_used_in_array_but_bad_alignof_global = common addrspace(200) global %struct.correct_when_used_in_array_but_bad_alignof zeroinitializer, align 1


struct bad_uintptr_array_1 { // expected-note-re {{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed,annotate("underaligned_capability")));
struct bad_uintptr_array_1 bad_uintptr_array_1_global;
// CHECK: @bad_uintptr_array_1_global = common addrspace(200) global %struct.bad_uintptr_array_1 zeroinitializer, align 1

struct bad_uintptr_array_2 { // expected-note-re {{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}} expected-warning-re{{alignment (8) of 'struct bad_uintptr_array_2' is less than the required capability alignment ({{16|32}})}}
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));
struct bad_uintptr_array_2 bad_uintptr_array_2_global;
// CHECK: @bad_uintptr_array_2_global = common addrspace(200) global %struct.bad_uintptr_array_2 zeroinitializer, align 8

struct good_uintptr_array {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));
struct good_uintptr_array good_uintptr_array_global;
// CHECK: @good_uintptr_array_global = common addrspace(200) global %struct.good_uintptr_array zeroinitializer, align 32


// same again but the member is a struct containing a capability
struct bad_cap_struct_array_1 { // expected-note-re{{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed,annotate("underaligned_capability")));
struct bad_cap_struct_array_1 bad_cap_struct_array_1_global;
// CHECK: @bad_cap_struct_array_1_global = common addrspace(200) global %struct.bad_cap_struct_array_1 zeroinitializer, align 1

struct bad_cap_struct_array_2 { // expected-note-re{{Add __attribute__((aligned({{16|32}}))) to ensure sufficient alignment}} expected-warning-re{{alignment (8) of 'struct bad_cap_struct_array_2' is less than the required capability alignment ({{16|32}})}}
  char before[sizeof(void*)];
  struct cap_struct cap; // expected-warning-re {{Capability field at offset {{16|32}} in packed structure will trap if structure is used in an array}}
  char after[1];
} __attribute__((packed)) __attribute__((aligned(8)));
struct bad_cap_struct_array_2 bad_cap_struct_array_2_global;
// CHECK: @bad_cap_struct_array_2_global = common addrspace(200) global %struct.bad_cap_struct_array_2 zeroinitializer, align 8

struct good_cap_struct_array {
  char before[sizeof(void*)];
  struct cap_struct cap;
  char after[1];
} __attribute__((packed)) __attribute__((aligned(32)));
struct good_cap_struct_array good_cap_struct_array_global;
// CHECK: @good_cap_struct_array_global = common addrspace(200) global %struct.good_cap_struct_array zeroinitializer, align 32

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
