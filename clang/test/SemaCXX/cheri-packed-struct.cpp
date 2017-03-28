// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi purecap
// Similar to Sema/packed-struct.c but also check dependent types

template <typename T>
struct packed_T_and_cap {
  T t;
  void* __capability cap; // expected-warning-re 3 {{Under aligned capability field at offset {{4|8|31}} in packed structure will trap if accessed}}
} __attribute__((packed));

struct inherit_from_template_good : public packed_T_and_cap<char[sizeof(void*)]> {
  char pad[sizeof(void*)];
};

struct inherit_from_template_bad : public packed_T_and_cap<char[sizeof(void*) - 1]> { // expected-note {{in instantiation of template class 'packed_T_and_cap<char [31]>' requested here}}
  char pad[sizeof(void*)];
};

struct inherit_from_template_bad_array : public packed_T_and_cap<__uintcap_t> {
  char bad;
} __attribute__((packed));

int main() {
  inherit_from_template_bad a;
  inherit_from_template_good b;
  inherit_from_template_bad_array c;
  packed_T_and_cap<int> d; // expected-note {{in instantiation of template class 'packed_T_and_cap<int>' requested here}}
  packed_T_and_cap<void*> e;
  packed_T_and_cap<long> f;  // expected-note {{in instantiation of template class 'packed_T_and_cap<long>' requested here}}
  return sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);
}
