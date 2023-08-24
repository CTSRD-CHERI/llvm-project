// RUN: %cheri_purecap_cc1 -fno-rtti -emit-llvm -mllvm -cheri-cap-table-abi=plt -o - %s | FileCheck %s -check-prefixes CHECK,CAPTABLE --implicit-check-not=cheri.pcc.get --implicit-check-not=addrspacecast

class A {
public:
  int nonvirt() { return 1; }
  int nonvirt2() { return 2; }
  virtual int virt() { return 3; }
  virtual int virt2() { return 4; }
};
int global_fn() { return 5; }

typedef int (A::* MemberPtr)();

MemberPtr global_nonvirt_ptr = &A::nonvirt;
MemberPtr global_virt_ptr = &A::virt;
int (*global_fn_ptr)() = &global_fn;

// CHECK: @global_nonvirt_ptr = addrspace(200) global { ptr addrspace(200), i64 } {
// CAPTABLE-SAME: ptr addrspace(200) @_ZN1A7nonvirtEv, i64 0 }, align 16
// CHECK:  @global_virt_ptr = addrspace(200) global { ptr addrspace(200), i64 } { ptr addrspace(200) null, i64 1 }, align 16
// CAPTABLE: @global_fn_ptr = addrspace(200) global ptr addrspace(200) @_Z9global_fnv, align 16

void local_struct_constant_init() {
  struct {
      MemberPtr p;
  } s{&A::nonvirt};
  // CHECK: @__const._Z26local_struct_constant_initv.s = private unnamed_addr addrspace(200) constant %struct.anon { { ptr addrspace(200), i64 } {
  // CAPTABLE: ptr addrspace(200) @_ZN1A7nonvirtEv, i64 0 } }, align 16
}

int call_nonvirt(A* a) {
  // CHECK-LABEL: @_Z12call_nonvirtP1A(
  // CHECK: load { ptr addrspace(200), i64 }, ptr addrspace(200) @global_nonvirt_ptr, align 16
  return (a->*global_nonvirt_ptr)();
}

int call_virt(A* a) {
  // CHECK-LABEL: @_Z9call_virtP1A(
  // CHECK: load { ptr addrspace(200), i64 }, ptr addrspace(200) @global_virt_ptr, align 16
  return (a->*global_virt_ptr)();
}

int call_local_nonvirt(A* a) {
  // CHECK-LABEL: @_Z18call_local_nonvirtP1A(
  MemberPtr local_nonvirt = &A::nonvirt2;
  // CAPTABLE: store { ptr addrspace(200), i64 } { ptr addrspace(200) @_ZN1A8nonvirt2Ev, i64 0 }, ptr addrspace(200) %local_nonvirt, align 16
  return (a->*local_nonvirt)();
}

int call_local_virt(A* a) {
  MemberPtr local_virt = &A::virt2;
  // CHECK-LABEL: @_Z15call_local_virtP1A(
  // CHECK: store { ptr addrspace(200), i64 } { ptr addrspace(200) getelementptr (i8, ptr addrspace(200) null, i64 16), i64 1 }, ptr addrspace(200) %local_virt, align 16
  return (a->*local_virt)();
}

int call_local_fn_ptr(A* a) {
  // CHECK-LABEL: @_Z17call_local_fn_ptrP1A(
  int (*local_fn_ptr)() = &global_fn;
  // CAPTABLE: store ptr addrspace(200) @_Z9global_fnv, ptr addrspace(200) %local_fn_ptr, align 16
  return local_fn_ptr();
}

int main() {
  A a;
  global_fn_ptr();
  call_virt(&a);
  call_nonvirt(&a);
  call_virt(&a);
}
