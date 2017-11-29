// RUN: %cheri_purecap_cc1 -std=c++11 -emit-llvm %s -o - | %cheri_FileCheck %s
template <class Functor>
void invoke(Functor&& f);

int& f(void);

void g() {
  // Skip the first call to llvm.cheri.pcc.get
  // CHECK: call {{.*}} @llvm.cheri.pcc.get() 
  int&(&fn)(void) = f;
  // CHECK: [[TMP1:%[0-9]]] = call {{.*}} @llvm.cheri.pcc.get() 
  // CHECK-NEXT: [[TMP2:%[0-9]]] = call {{.*}} @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[TMP1]]
  // CHECK-NEXT: [[TMP3:%[0-9]]] = bitcast i8 addrspace(200)* [[TMP2]]
  // CHECK-NEXT: call void @_Z6invokeIU3capRFU3capRivEEvU3capOT_({{.*}} [[TMP3]])
  invoke(fn);
}
