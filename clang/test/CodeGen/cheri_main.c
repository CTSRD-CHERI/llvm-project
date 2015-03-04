// RUN: %clang -DNO_ARGS=1 -target cheri-unknown-freebsd %s -o - -emit-llvm -S | FileCheck %s
// RUN: %clang -DPTR_PTR=1 -target cheri-unknown-freebsd %s -o - -emit-llvm -S | FileCheck %s
// RUN: %clang -DCONST_PTR_PTR=1 -target cheri-unknown-freebsd %s -o - -emit-llvm -S | FileCheck %s
// RUN: %clang -DARR_PTR=1 -target cheri-unknown-freebsd %s -o - -emit-llvm -S | FileCheck %s
// RUN: %clang -DCONST_ARR_PTR=1 -target cheri-unknown-freebsd %s -o - -emit-llvm -S | FileCheck %s
// CHECK: main

// Ensure various legal main function protoypes are accepted.

#ifdef NO_ARGS
int main() {
  return 0;
}
#endif

#ifdef PTR_PTR
int main(int argc, char **argv) {
  return 0;
}
#endif

#ifdef CONST_PTR_PTR
int main(int argc, const char * const *argv) {
  return 0;
}
#endif

#ifdef ARR_PTR
int main(int argc, char* argv[]) {
  return 0;
}
#endif

#ifdef CONST_ARR_PTR
int main(int argc, const char* argv[]) {
  return 0;
}
#endif
