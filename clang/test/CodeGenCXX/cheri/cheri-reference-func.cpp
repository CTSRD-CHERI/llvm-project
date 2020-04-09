// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=c++11 -emit-llvm %s -o - | %cheri_FileCheck %s -check-prefixes CHECK,LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c++11 -emit-llvm %s -o - | %cheri_FileCheck %s -check-prefixes CHECK,NEWABI
template <class Functor>
void invoke(Functor&& f);

int& f(void);

void g() {
  int&(&fn)(void) = f;
  // LEGACY: [[TMP1:%[0-9]]] = call {{.*}} @llvm.cheri.pcc.get()
  // LEGACY-NEXT: [[TMP2:%[0-9]]] = call {{.*}} @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* [[TMP1]]
  // LEGACY-NEXT: [[TMP3:%[0-9]]] = bitcast i8 addrspace(200)* [[TMP2]]
  // LEGACY-NEXT: call void @_Z6invokeIRFRivEEvOT_({{.*}} [[TMP3]])
  // NEWABI: call void @_Z6invokeIRFRivEEvOT_(i32 addrspace(200)* () addrspace(200)* @_Z1fv)
  invoke(fn);
}
