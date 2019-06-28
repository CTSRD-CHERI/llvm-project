// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,NEWABI

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=c++11 -fcxx-exceptions -fexceptions -o - -O2 -S %s | %cheri_FileCheck %s -check-prefixes ASM
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -fcxx-exceptions -fexceptions -o - -O2 -S %s | %cheri_FileCheck %s -check-prefixes ASM

// CHECK: @_ZTIi = external addrspace(200) constant i8 addrspace(200)*

extern void fn2(int);

// LEGACY: define void @_Z3fn1v() {{.+}} personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
// NEWABI: define void @_Z3fn1v() {{.+}} personality i8 addrspace(200)* bitcast (i32 (...) addrspace(200)* @__gxx_personality_v0 to i8 addrspace(200)*)
void fn1() {
  try {
    throw 4;
  } catch (int x) {
    fn2(x);
  }
}

// ASM: .L_ZTIi.DW.stub:
// ASM:         .chericap       _ZTIi

// CHECK: declare i32 @__gxx_personality_v0(...)

// ASM: .type	DW.ref.__gxx_personality_v0,@object
// ASM: .size	DW.ref.__gxx_personality_v0, [[#CAP_SIZE]]
// ASM: DW.ref.__gxx_personality_v0:
// ASM:	.chericap	__gxx_personality_v0
