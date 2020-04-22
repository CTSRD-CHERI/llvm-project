// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -emit-llvm %s -o - | %cheri_FileCheck %s -check-prefixes CHECK,NEWABI
template <class Functor>
void invoke(Functor&& f);

int& f(void);

void g() {
  int&(&fn)(void) = f;
  // CHECK: call void @_Z6invokeIRFRivEEvOT_(i32 addrspace(200)* () addrspace(200)* @_Z1fv)
  invoke(fn);
}
