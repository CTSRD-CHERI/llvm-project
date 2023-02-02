// RUN: %clang_cc1 %s -o - "-triple" "riscv32-unknown-unknown" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-cpu" "cheriot" "-target-feature" "+xcheri" "-target-feature" "-64bit" "-target-feature" "-relax" "-target-feature" "-xcheri-rvc" "-target-feature" "-save-restore" "-target-abi" "cheriot" "-Oz" "-Wno-atomic-alignment" "-cheri-compartment=example" | FileCheck %s

_Atomic(int) x;

int callFromNotLibcall(void) {
  // Check that atomic libcalls get the right calling convention at the call site.
  // CHECK: call cherilibcallcc
  return __c11_atomic_fetch_add(&x, 1, 5);
}
