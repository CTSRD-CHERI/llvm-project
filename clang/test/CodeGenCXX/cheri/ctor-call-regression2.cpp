// This was previously asserting in TryEmitBaseDestructorAsAlias()
// RUN: %cheri_purecap_cc1 -mconstructor-aliases -mllvm -cheri-cap-table-abi=legacy -O2 -std=c++11 -fcxx-exceptions -fexceptions -o - -emit-llvm %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -mconstructor-aliases -mllvm -cheri-cap-table-abi=pcrel -O2 -std=c++11 -fcxx-exceptions -fexceptions -o - -emit-llvm %s | FileCheck %s

class a {
public:
  static a b;
};
class c {
public:
  ~c();
};
class C : c {
public:
  C(int *, a);
};
int d;
void e() { C(&d, a::b); }

// C::C(int* cap, a):
// CHECK: declare void @_ZN1CC1EU3capPi1a(%class.C addrspace(200)*, i32 addrspace(200)*, i8 inreg) unnamed_addr

