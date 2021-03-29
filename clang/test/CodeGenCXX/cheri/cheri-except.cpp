// REQUIRES: mips-registered-target
// REQUIRES: riscv-registered-target
// RUN: %cheri_purecap_cc1 -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK
// RUN: %cheri_purecap_cc1 -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -S %s | %cheri_FileCheck %s -check-prefixes ASM
// RUN: %riscv64_cheri_purecap_cc1 -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -S %s | %cheri_FileCheck %s -check-prefixes ASM
/// Check that the __gxx_personality_v0 reference and DWARF stub symbols use capabilities and not an addresses

// CHECK: @_ZTIi = external addrspace(200) constant i8 addrspace(200)*
extern void fn2(int);

// CHECK: define dso_local void @_Z3fn1v() {{.+}} personality i8 addrspace(200)* bitcast (i32 (...) addrspace(200)* @__gxx_personality_v0 to i8 addrspace(200)*)
void fn1() {
  try {
    throw 4;
  } catch (int x) {
    fn2(x);
  }
}

// CHECK: declare i32 @__gxx_personality_v0(...)

// ASM-LABEL: # -- End function
// ASM-NEXT:  .data{{$}}
// ASM-NEXT:  .p2align	4
// ASM-NEXT:  .L_ZTIi.DW.stub:
// ASM-NEXT:   .chericap       _ZTIi
// ASM-LABEL: .hidden	DW.ref.__gxx_personality_v0
// ASM-NEXT:  .weak	DW.ref.__gxx_personality_v0
// ASM-NEXT:  .section .data.DW.ref.__gxx_personality_v0,"aGw",@progbits,DW.ref.__gxx_personality_v0,comdat
// ASM-NEXT:  .p2align 4
// ASM-NEXT:  .type DW.ref.__gxx_personality_v0,@object
// ASM-NEXT:  .size DW.ref.__gxx_personality_v0, [[#CAP_SIZE]]
// ASM-NEXT:  DW.ref.__gxx_personality_v0:
// ASM-NEXT:   .chericap __gxx_personality_v0
