// RUN: %cheri_cc1 -o - -O2 -emit-llvm %s | FileCheck %s

void* __capability add_cap(void) {
  char* __capability cap = (char* __capability)100;
  cap += 10;
  return cap + 914;
  // This can return a valid tagged capability:
  // CHECK-LABEL: @add_cap()
  // CHECK: ret i8 addrspace(200)* inttoptr (i64 1024 to i8 addrspace(200)*)
}

__uintcap_t add_uintcap_t(void) {
  __uintcap_t cap = (__uintcap_t)100;
  cap += 10;
  return cap + 914;
  // But this always returns a untagged capability with offset 1024
  // CHECK-LABEL: @add_uintcap_t()
  // CHECK: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 1024)
}
