// RUN: %cheri_purecap_cc1 -fno-rtti -emit-llvm -cheri-linker -mllvm -cheri-cap-table-abi=legacy -o - %s | %cheri_FileCheck %s -check-prefixes CHECK,LEGACY
// RUN: %cheri_purecap_cc1 -fno-rtti -emit-llvm -cheri-linker -mllvm -cheri-cap-table-abi=plt -o - %s | %cheri_FileCheck %s -check-prefixes CHECK,CAPTABLE -implicit-check-not=cheri.pcc.get
// RUN: %cheri_purecap_cc1 -fno-rtti -emit-obj -cheri-linker -mllvm -cheri-cap-table-abi=legacy -o - %s | llvm-readobj -r - | %cheri_FileCheck -check-prefix=RELOCS %s
// RUN: %cheri_purecap_cc1 -fno-rtti -emit-obj -cheri-linker -mllvm -cheri-cap-table-abi=plt -o - %s | llvm-readobj -r - | %cheri_FileCheck -check-prefix=RELOCS %s

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

// CHECK: @global_nonvirt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } {
// LEGACY-SAME: i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A7nonvirtEv to i8*) to i8 addrspace(200)*), i64 0 }, align [[#CAP_SIZE]]
// CAPTABLE-SAME: i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A7nonvirtEv to i8 addrspace(200)*), i64 0 }, align [[#CAP_SIZE]]
// CHECK: @global_virt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* null, i64 1 }, align [[#CAP_SIZE]]
// LEGACY: @global_fn_ptr = addrspace(200) global i32 () addrspace(200)* addrspacecast (i32 ()* @_Z9global_fnv to i32 () addrspace(200)*), align [[#CAP_SIZE]]
// CAPTABLE: @global_fn_ptr = addrspace(200) global i32 () addrspace(200)* @_Z9global_fnv, align [[#CAP_SIZE]]

void local_struct_constant_init() {
  struct {
      MemberPtr p;
  } s{&A::nonvirt};
  // CHECK: @__const._Z26local_struct_constant_initv.s = private unnamed_addr addrspace(200) constant %struct.anon { { i8 addrspace(200)*, i64 } {
  // LEGACY: i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A7nonvirtEv to i8*) to i8 addrspace(200)*), i64 0 } }, align [[#CAP_SIZE]]
  // CAPTABLE: i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A7nonvirtEv to i8 addrspace(200)*), i64 0 } }, align [[#CAP_SIZE]]
}

int call_nonvirt(A* a) {
  // CHECK-LABEL: @_Z12call_nonvirtU3capP1A(
  // CHECK: load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* @global_nonvirt_ptr, align [[#CAP_SIZE]]
  return (a->*global_nonvirt_ptr)();
}

int call_virt(A* a) {
  // CHECK-LABEL: @_Z9call_virtU3capP1A(
  // CHECK: load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* @global_virt_ptr, align [[#CAP_SIZE]]
  return (a->*global_virt_ptr)();
}

int call_local_nonvirt(A* a) {
  // CHECK-LABEL: @_Z18call_local_nonvirtU3capP1A(
  MemberPtr local_nonvirt = &A::nonvirt2;
  // FIXME: should we rather memcopy from a global that has been initialized?
  // LEGACY: call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // LEGACY: call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %{{.+}}, i64 ptrtoint (i32 (%class.A addrspace(200)*)* @_ZN1A8nonvirt2Ev to i64))
  // For cap-table that should already be true:
  // CAPTABLE: store i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A8nonvirt2Ev to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)*
  return (a->*local_nonvirt)();
}

int call_local_virt(A* a) {
  MemberPtr local_virt = &A::virt2;
  // CHECK-LABEL: @_Z15call_local_virtU3capP1A(
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 [[#CAP_SIZE]] to i8 addrspace(200)*), i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)*
  return (a->*local_virt)();
}

int call_local_fn_ptr(A* a) {
  // CHECK-LABEL: @_Z17call_local_fn_ptrU3capP1A(
  int (*local_fn_ptr)() = &global_fn;
  // LEGACY: call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // LEGACY: call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %{{.+}}, i64 ptrtoint (i32 ()* @_Z9global_fnv to i64))
  // CAPTABLE: store i32 () addrspace(200)* @_Z9global_fnv, i32 () addrspace(200)* addrspace(200)*
  return local_fn_ptr();
}

int main() {
  A a;
  global_fn_ptr();
  call_virt(&a);
  call_nonvirt(&a);
  call_virt(&a);
}

// RELOCS:      Section (22) .rela.data {
// RELOCS-NEXT:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _ZN1A7nonvirtEv 0x0
// RELOCS-NEXT:   0x{{4|8}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z9global_fnv 0x0
// RELOCS-NEXT: }
// RELOCS-NEXT: Section (24) .rela.data.rel.ro {
// RELOCS-NEXT:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _ZN1A7nonvirtEv 0x0
// RELOCS-NEXT: }
// RELOCS-NEXT: Section (27) .rela.data.rel.ro._ZTV1A {
// RELOCS-NEXT:   0x{{2|4}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _ZN1A4virtEv 0x0
// RELOCS-NEXT:   0x{{3|6}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _ZN1A5virt2Ev 0x0
// RELOCS-NEXT: }
