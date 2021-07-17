// REQUIRES: mips-registered-target

// RUN: %cheri_cc1 -o /dev/null -O0 -S %s -verify
// expected-no-diagnostics

// This used to warn about the inefficient memcpy (but in fact it is just too big to be copied inline)
void * memcpy(void *, const void *, unsigned long);
void * memmove(void *, const void *, unsigned long);

struct libcheri_stack_frame {
  void * __capability lcsf_caller_pcc;
  void * __capability lcsf_caller_csp;
};
struct libcheri_stack {
  struct libcheri_stack_frame lcs_frames[8];
} __libcheri_stack_tls_storage;

struct libcheri_stack* test1(struct libcheri_stack* buffer) {
  memcpy(buffer, &__libcheri_stack_tls_storage, sizeof(*buffer));
  return buffer;
}

void test2(void* buffer) {
  // This used to assert (size < cap_size && unaligned):
  memcpy(buffer, &__libcheri_stack_tls_storage, 1);
}

void test3(struct libcheri_stack* buffer) {
  // This used to assert (size < cap_size && aligned):
  memcpy(buffer, &__libcheri_stack_tls_storage, 1);
}


// Same again with memmove:
struct libcheri_stack* test4(struct libcheri_stack* buffer) {
  memmove(buffer, &__libcheri_stack_tls_storage, sizeof(*buffer));
  return buffer;
}

void test5(void* buffer) {
  // This used to assert (size < cap_size && unaligned):
  memmove(buffer, &__libcheri_stack_tls_storage, 1);
}

void test6(struct libcheri_stack* buffer) {
  // This used to assert (size < cap_size && aligned):
  memmove(buffer, &__libcheri_stack_tls_storage, 1);
}
