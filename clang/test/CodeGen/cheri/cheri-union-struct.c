// RUN: %cheri_purecap_cc1 %s -O0 -emit-llvm -o - | %cheri_FileCheck %s
union CallData {
  struct {
    char* c;
    int i;
  } Bar;
};

void f(union CallData foo) {
  // CHECK: [[TMP0:%[a-zA-Z0-9]+]] = alloca %union.CallData, align [[$CAP_SIZE]], addrspace(200)
  // CHECK-NEXT: [[TMP1:%[0-9]]] = bitcast i8 addrspace(200)* {{.*}}.coerce to %union.CallData addrspace(200)*
  // CHECK-NEXT: [[TMP2:%[0-9]]] = load %union.CallData, %union.CallData addrspace(200)* [[TMP1]], align [[$CAP_SIZE]]
  // CHECK-NEXT: store %union.CallData [[TMP2]], %union.CallData addrspace(200)* [[TMP0]], align [[$CAP_SIZE]]
}

void g() {
  union CallData foo;
  foo.Bar.c = "hello";
  foo.Bar.i = 9;
	// CHECK: [[TMP3:%[0-9]]] = bitcast %union.CallData addrspace(200)* {{.*}} to i8 addrspace(200)*
  // CHECK-NEXT: call void @f(i8 addrspace(200)* inreg [[TMP3]])
  f(foo);
}
