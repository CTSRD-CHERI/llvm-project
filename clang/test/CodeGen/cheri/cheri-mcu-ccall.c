// RUN: %clang_cc1 %s -o - "-triple" "riscv32-unknown-unknown" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-cpu" "cheriot" "-target-feature" "+xcheri" "-target-feature" "-64bit" "-target-feature" "-relax" "-target-feature" "-xcheri-rvc" "-target-feature" "-save-restore" "-target-abi" "cheriot" "-Oz" "-Werror" "-cheri-compartment=example" | FileCheck %s

// CHECK: define dso_local chericcallcce i32 @_Z5test2ii(i32 %a0, i32 %a1) local_unnamed_addr addrspace(200) #0
__attribute__((cheri_compartment("example"))) int test2(int a0, int a1) {
  return a0 + a1;
}

__attribute__((cheri_compartment("other"))) int test6callee(int *a0, int *a1, int *a2, int *a3, int *a4, int *a5);

int testcall6() {
  static int stack_arg;
  int args[8];
  // CHECK: call chericcallcc i32 @_Z11test6calleePiS_S_S_S_S_
  return test6callee(
      &args[0],
      &args[1],
      &args[2],
      &args[3],
      &args[4],
      &stack_arg);
}

// CHECK: declare chericcallcc i32 @_Z11test6calleePiS_S_S_S_S_(i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) #3

// CHECK: attributes #0 = {
// CHECK-SAME: "cheri-compartment"="example"
// CHECK: attributes #3 = {
// CHECK-SAME: "cheri-compartment"="other"
