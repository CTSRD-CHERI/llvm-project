// RUN: %clang -target cheri-unknown-freebsd -mabi=purecap -S -emit-llvm -o - %s | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=purecap -c %s -o %t.obj
// RUN: llvm-objdump -C -r %t.obj | FileCheck -check-prefix RELOCS-OBJ %s

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

// CHECK: @global_nonvirt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A7nonvirtEv to i8*) to i8 addrspace(200)*), i64 0 }, align 8
// CHECK: @global_virt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* null, i64 1 }, align 8
// CHECK: @global_fn_ptr = addrspace(200) global i32 () addrspace(200)* addrspacecast (i32 ()* @_Z9global_fnv to i32 () addrspace(200)*), align 32

int call_nonvirt(A* a) {
  return (a->*global_nonvirt_ptr)();
}

int call_virt(A* a) {
  return (a->*global_nonvirt_ptr)();
}

int call_local_nonvirt(A* a) {
  MemberPtr local_nonvirt = &A::nonvirt2;
  return (a->*local_nonvirt)();
}

int call_local_virt(A* a) {
  MemberPtr local_virt = &A::virt2;
  return (a->*local_virt)();
}

int call_local_fn_ptr(A* a) {
int (*local_fn_ptr)() = &global_fn;
  return local_fn_ptr();
}

int main() {
  A a;
  global_fn_ptr();
  call_virt(&a);
  call_nonvirt(&a);
  call_virt(&a);
}

// RELOCS-OBJ: RELOCATION RECORDS FOR [.rela__cap_relocs]:
// RELOCS-OBJ: 0000000000000000 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data
// RELOCS-OBJ: 0000000000000008 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE _ZN1A7nonvirtEv
// RELOCS-OBJ: 0000000000000028 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data
// RELOCS-OBJ: 0000000000000030 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE _Z9global_fnv
// RELOCS-OBJ: 0000000000000050 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro._ZTV1A
// RELOCS-OBJ: 0000000000000058 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE _ZN1A4virtEv
// RELOCS-OBJ: 0000000000000078 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro._ZTV1A
// RELOCS-OBJ: 0000000000000080 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE _ZN1A5virt2Ev

// RELOCS-OBJ: CAPABILITY RELOCATION RECORDS:
// RELOCS-OBJ: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0000000000000000        Length: 0000000000000000        Permissions: 00000000
// RELOCS-OBJ: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0000000000000000        Length: 0000000000000000        Permissions: 00000000
// RELOCS-OBJ: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0000000000000000        Length: 0000000000000000        Permissions: 00000000
// RELOCS-OBJ: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0000000000000000        Length: 0000000000000000        Permissions: 00000000

