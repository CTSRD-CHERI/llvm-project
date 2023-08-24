// RUN:  %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm -std=c++11 -o - %s | FileCheck %s

// CHECK: @llvm.global_ctors = appending addrspace(200) global [1 x { i32, ptr addrspace(200), ptr addrspace(200) }] [{ i32, ptr addrspace(200), ptr addrspace(200) } { i32 65535, ptr addrspace(200) @_GLOBAL__sub_I_cheri_destructor_atexit.cpp, ptr addrspace(200) null }]

// CHECK: define internal void @__cxx_global_var_init()
// CHECK:   [[TMP0:%[0-9]+]] = call i32 @__cxa_atexit(
// CHECK-SAME: ptr addrspace(200) @_ZN1AD1Ev,
// CHECK-SAME: ptr addrspace(200) @a,
// CHECK-SAME: ptr addrspace(200) @__dso_handle) #2
// CHECK-NEXT:   ret void

class A {
  public:
    ~A() { }
};

A a;

// CHECK-LABEL: @main(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[RETVAL:%.*]] = alloca i32, align 4, addrspace(200)
// CHECK-NEXT:    store i32 0, ptr addrspace(200) [[RETVAL]], align 4
// CHECK-NEXT:    ret i32 0
//
int main(void) {
  return 0;
}
