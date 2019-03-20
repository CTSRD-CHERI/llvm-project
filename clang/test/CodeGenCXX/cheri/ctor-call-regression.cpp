// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -fcxx-exceptions -fexceptions -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK,NEWABI

// Also check that we can emit assembly code without asserting:
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=c++11 -fcxx-exceptions -fexceptions -o - -O2 -S %s | FileCheck %s -check-prefixes ASM
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -fcxx-exceptions -fexceptions -o - -O2 -S %s | FileCheck %s -check-prefixes ASM

class a {
public:
  static a b;
};
class c {
public:
  c(int *, a);
};
int d;
// LEGACY: define void @_Z3fn1v() {{.+}} personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
// NEWABI: define void @_Z3fn1v() {{.+}} personality i8 addrspace(200)* bitcast (i32 (...) addrspace(200)* @__gxx_personality_v0 to i8 addrspace(200)*)
void fn1() {
  // Invoke c::c(int* cap, a)
  // CHECK:       invoke void @_ZN1cC1EU3capPi1a(%class.c addrspace(200)* %0, i32 addrspace(200)* @d, i8 inreg %3)
  // CHECK-NEXT:  to label %[[INVOKE_CONT:.+]] unwind label %lpad
  // CHECK: [[INVOKE_CONT]]
  // CHECK-NEXT: ret void
  new c(&d, a::b);
}

// CHECK: declare i32 @__gxx_personality_v0(...)

// ASM: .type	DW.ref.__gxx_personality_v0,@object
// ASM: .size	DW.ref.__gxx_personality_v0, 8
// ASM: DW.ref.__gxx_personality_v0:
// ASM:	.8byte	__gxx_personality_v0
