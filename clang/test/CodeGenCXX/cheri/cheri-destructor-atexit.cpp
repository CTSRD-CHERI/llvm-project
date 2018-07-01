// RUN:  %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-llvm -std=c++11 -o - %s | FileCheck %s
// RUN:  %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | FileCheck %s -check-prefix NEWABI

// CHECK-LABEL: define internal void @__cxx_global_var_init()
// CHECK: [[TMP0:%[0-9]+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
// CHECK-NEXT: [[TMP1:%[0-9]+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[TMP0]], i64 ptrtoint (void (%class.A addrspace(200)*)* @{{.*}} i64))
// CHECK-NEXT: [[TMP2:%[0-9]+]] = bitcast i8 addrspace(200)* [[TMP1]] to void (i8 addrspace(200)*) addrspace(200)*
// CHECK-NEXT: [[TMP3:%[0-9]+]] = call i32 @__cxa_atexit(void (i8 addrspace(200)*) addrspace(200)* [[TMP2]], i8 addrspace(200)* getelementptr inbounds (%class.A, %class.A addrspace(200)* @a, i32 0, i32 0), i8 addrspace(200)* @__dso_handle)


// NEWABI: define internal void @__cxx_global_var_init()
// NEWABI:   [[TMP0:%[0-9]+]] = call i32 @__cxa_atexit(
// NEWABI-SAME: void (i8 addrspace(200)*) addrspace(200)* bitcast (void (%class.A addrspace(200)*) addrspace(200)* @_ZN1AD1Ev to void (i8 addrspace(200)*) addrspace(200)*),
// NEWABI-SAME: i8 addrspace(200)* getelementptr inbounds (%class.A, %class.A addrspace(200)* @a, i32 0, i32 0),
// NEWABI-SAME: i8 addrspace(200)* @__dso_handle) #2
// NEWABI-NEXT:   ret void

class A {
  public:
    ~A() { }
};

A a;

int main(void) {
  return 0;
}
