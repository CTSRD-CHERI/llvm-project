// RUN:  %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm -std=c++11 -o - %s | FileCheck %s


// CHECK: define internal void @__cxx_global_var_init()
// CHECK:   [[TMP0:%[0-9]+]] = call i32 @__cxa_atexit(
// CHECK-SAME: void (i8 addrspace(200)*) addrspace(200)* bitcast (void (%class.A addrspace(200)*) addrspace(200)* @_ZN1AD1Ev to void (i8 addrspace(200)*) addrspace(200)*),
// CHECK-SAME: i8 addrspace(200)* getelementptr inbounds (%class.A, %class.A addrspace(200)* @a, i32 0, i32 0),
// CHECK-SAME: i8 addrspace(200)* @__dso_handle) #2
// CHECK-NEXT:   ret void

class A {
  public:
    ~A() { }
};

A a;

// CHECK-LABEL: define {{[^@]+}}@main() addrspace(200) #3
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[RETVAL:%.*]] = alloca i32, align 4, addrspace(200)
// CHECK-NEXT:    store i32 0, i32 addrspace(200)* [[RETVAL]], align 4
// CHECK-NEXT:    ret i32 0
//
int main(void) {
  return 0;
}
