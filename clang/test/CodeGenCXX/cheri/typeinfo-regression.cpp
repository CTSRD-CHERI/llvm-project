// RUN: %cheri_purecap_cc1 -mconstructor-aliases -mllvm -cheri-cap-table-abi=pcrel -nostdinc++ -O2 -std=c++11 -o - -emit-llvm %s | FileCheck %s
namespace a {
class b {
public:
  ~b();
};
}
struct c : a::b {
  ~c();
};
struct C : c {
  ~C();
};
using a::b;
b::~b() {}
c::~c() {}
C::~C() {}

// CHECK: @_ZN1a1bD1Ev = dso_local unnamed_addr alias void (%"class.a::b" addrspace(200)*), void (%"class.a::b" addrspace(200)*) addrspace(200)* @_ZN1a1bD2Ev
// CHECK: @_ZN1cD2Ev = dso_local unnamed_addr alias void (%struct.c addrspace(200)*), bitcast (void (%"class.a::b" addrspace(200)*) addrspace(200)* @_ZN1a1bD2Ev to void (%struct.c addrspace(200)*) addrspace(200)*)
// CHECK: @_ZN1cD1Ev = dso_local unnamed_addr alias void (%struct.c addrspace(200)*), bitcast (void (%"class.a::b" addrspace(200)*) addrspace(200)* @_ZN1a1bD2Ev to void (%struct.c addrspace(200)*) addrspace(200)*)
// CHECK: @_ZN1CD2Ev = dso_local unnamed_addr alias void (%struct.C addrspace(200)*), bitcast (void (%"class.a::b" addrspace(200)*) addrspace(200)* @_ZN1a1bD2Ev to void (%struct.C addrspace(200)*) addrspace(200)*)
// CHECK: @_ZN1CD1Ev = dso_local unnamed_addr alias void (%struct.C addrspace(200)*), bitcast (void (%"class.a::b" addrspace(200)*) addrspace(200)* @_ZN1a1bD2Ev to void (%struct.C addrspace(200)*) addrspace(200)*)
