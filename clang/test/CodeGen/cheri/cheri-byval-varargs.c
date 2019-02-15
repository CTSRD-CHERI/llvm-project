// RUN:  %cheri128_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-x" "c" %s -o - -emit-llvm | FileCheck %s
// RUN:  %cheri256_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-x" "c" %s -o - -emit-llvm | FileCheck %s
// RUN:  %cheri128_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-x" "c" %s -o - | FileCheck %s
// RUN:  %cheri256_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-x" "c" %s -o - | FileCheck %s
// CHECK-NOT: safasdg
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
int varargs_fn();
int val_fn(Dwarf_Error a);
int ptr_fn(Dwarf_Error* a);

int main() {
  ptr_fn(&a);
  val_fn(a);
  varargs_fn(a);
  undefined_fn(a);
  return 0;
}
