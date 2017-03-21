// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi sandbox
// Ensure that we warn on structs packed structs with misaligned capabilities

struct padded_cap {
  int i;
  void* __capability cap;
};

struct packed_cap {
  int i;
  void* __capability cap;  // expected-warning {{Under aligned capability field in packed structure will trap if accessed}}
} __attribute__((packed));

struct padded_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;
};

struct packed_uintptr_t {
  int i;
  __UINTPTR_TYPE__ cap;  // expected-warning {{Under aligned capability field in packed structure will trap if accessed}}
} __attribute__((packed));

struct correctly_aligned_but_size_not_multiple {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
};

// TODO: we should probably also warn if this struct is used in an array
struct packed_correctly_aligned_but_size_not_multiple {
  char before[sizeof(void*)];
  __UINTPTR_TYPE__ cap;
  char after[1];
} __attribute__((packed));

int main() {
  struct padded_cap a;
  struct packed_cap b;
  struct padded_uintptr_t c;
  struct packed_uintptr_t d;
  struct correctly_aligned_but_size_not_multiple e;
  struct packed_correctly_aligned_but_size_not_multiple f;

  struct packed_correctly_aligned_but_size_not_multiple bad_array[2];

  return sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + sizeof(e) + sizeof(f);
}
