// REQUIRES: mips-registered-target

// RUN: %cheri128_purecap_cc1 "-emit-obj" "-O2" "-x" "c" %s -o /dev/null -emit-llvm -verify
// RUN: %cheri256_purecap_cc1 "-emit-obj" "-O2" "-x" "c" %s -o /dev/null -emit-llvm -verify
// RUN: %cheri128_purecap_cc1 "-emit-obj" "-O2" "-x" "c" %s -o /dev/null -S -verify
// RUN: %cheri256_purecap_cc1 "-emit-obj" "-O2" "-x" "c" %s -o /dev/null -S -verify

void other_func(void* x);

#define TEST_BYVAL(count, type)                                         \
  struct arg_##count##_##type { type x[count]; };                       \
  struct arg_##count##_##type global_##count##_##type##_struct;         \
  void take_##count##_##type##_byval(struct arg_##count##_##type arg);  \
  void call_##count##_##type##_byval(void) {                            \
    other_func(&global_##count##_##type##_struct);;                     \
    take_##count##_##type##_byval(global_##count##_##type##_struct);    \
  }

TEST_BYVAL(1, long)
TEST_BYVAL(2, long)
TEST_BYVAL(7, long)
TEST_BYVAL(8, long)
TEST_BYVAL(9, long)
TEST_BYVAL(1000, long)
TEST_BYVAL(1, __intcap_t)
TEST_BYVAL(2, __intcap_t)
TEST_BYVAL(7, __intcap_t)
TEST_BYVAL(8, __intcap_t)
TEST_BYVAL(9, __intcap_t)
TEST_BYVAL(1000, __intcap_t)

// This is the original reduced test case:
typedef struct { int err_msg[1024]; } Dwarf_Error;
Dwarf_Error a;
int varargs_fn(); // expected-note{{candidate function declaration needs parameter types}}
int val_fn(Dwarf_Error a);
int ptr_fn(Dwarf_Error* a);

int main() {
  ptr_fn(&a);
  val_fn(a);
  varargs_fn(a); // expected-warning{{call to function 'varargs_fn' with no prototype may lead to run-time stack corruption}}
  // expected-note@-1{{Calling functions without prototypes is dangerous}}
  undefined_fn(a); // expected-warning{{implicit declaration of function 'undefined_fn' is invalid in C99}}
  // expected-warning@-1{{call to function 'undefined_fn' with no prototype may lead to run-time stack corruption}}
  // expected-note@-2{{Calling functions without prototypes is dangerous}}
  // expected-note@-3{{candidate function declaration needs parameter types}}

  return 0;
}
