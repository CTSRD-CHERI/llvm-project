// RUN: %cheri_purecap_cc1 %s -O0 -emit-llvm -o - | %cheri_FileCheck %s

// example union from WebKit
union CallData {
  struct {
    char* c;
    int i;
  } Bar;
};

// example union that is passed into semctl()
union semun {
  int val;
  char *buf1;
  char *buf2;
  char *buf3;
};

// example of nested structs
union DeepDive {
  struct {
    struct {
      char* str;
		} B;
  } A;
};

union DeepNoDive {
  struct {
    struct {
      char* str;
      int d;
		} B;
  } A;
};

void test_onelevel_helper(union CallData foo) {
  // CHECK: [[TMP0:%[a-zA-Z0-9]+]] = alloca %union.CallData, align [[$CAP_SIZE]], addrspace(200)
  // CHECK-NEXT: [[TMP1:%[0-9]]] = bitcast i8 addrspace(200)* {{.*}}.coerce to %union.CallData addrspace(200)*
  // CHECK-NEXT: [[TMP2:%[0-9]]] = load %union.CallData, %union.CallData addrspace(200)* [[TMP1]], align [[$CAP_SIZE]]
  // CHECK-NEXT: store %union.CallData [[TMP2]], %union.CallData addrspace(200)* [[TMP0]], align [[$CAP_SIZE]]
}

void test_onelevel() {
  union CallData foo;
  foo.Bar.c = "hello";
  foo.Bar.i = 9;
	// CHECK: [[TMP3:%[0-9]]] = bitcast %union.CallData addrspace(200)* {{.*}} to i8 addrspace(200)*
  // CHECK-NEXT: call void @test_onelevel_helper(i8 addrspace(200)* inreg [[TMP3]])
  test_onelevel_helper(foo);
}

void test_zerolevels_helper(union semun s) {
  // CHECK: [[TMP4:%[a-zA-Z.]+]] = getelementptr inbounds %union.semun, %union.semun addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: store i8 addrspace(200)* {{.*}}, i8 addrspace(200)* addrspace(200)* [[TMP4]], align [[$CAP_SIZE]] 
}

void test_zerolevels() {
  union semun s;
  s.buf1 = "hello";
  // CHECK: [[TMP5:%[a-zA-Z.]+]] = getelementptr inbounds %union.semun, %union.semun addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP6:%[0-9]+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[TMP5]], align [[$CAP_SIZE]] 
  // CHECK-NEXT: call void @test_zerolevels_helper(i8 addrspace(200)* inreg [[TMP6]])
  test_zerolevels_helper(s);
}

void test_twolevels_dive_helper(union DeepDive d) {
  // CHECK: [[TMP7:%[a-zA-Z0-9.]+]] = getelementptr inbounds %union.DeepDive, %union.DeepDive addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP8:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.0, %struct.anon.0 addrspace(200)* [[TMP7]], i32 0, i32 0
  // CHECK-NEXT: [[TMP9:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.1, %struct.anon.1 addrspace(200)* [[TMP8]], i32 0, i32 0
  // CHECK-NEXT: store i8 addrspace(200)* {{.*}}, i8 addrspace(200)* addrspace(200)* [[TMP9]], align [[$CAP_SIZE]]
}

void test_twolevels_dive() {
  union DeepDive d;
  d.A.B.str = "hello";
  // CHECK: [[TMP10:%[a-zA-Z0-9.]+]] = getelementptr inbounds %union.DeepDive, %union.DeepDive addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP11:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.0, %struct.anon.0 addrspace(200)* [[TMP10]], i32 0, i32 0
  // CHECK-NEXT: [[TMP12:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.1, %struct.anon.1 addrspace(200)* [[TMP11]], i32 0, i32 0
  // CHECK-NEXT: [[TMP13:%[0-9]+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[TMP12]], align [[$CAP_SIZE]] 
  // CHECK-NEXT: call void @test_twolevels_dive_helper(i8 addrspace(200)* inreg [[TMP13]])
  test_twolevels_dive_helper(d);
}

void test_twolevels_nodive_helper(union DeepNoDive d) {
  // CHECK: [[TMP14:%[a-zA-Z0-9]+]] = alloca %union.DeepNoDive, align [[$CAP_SIZE]], addrspace(200)
  // CHECK-NEXT: [[TMP15:%[0-9]+]] = bitcast i8 addrspace(200)* {{.*}} to %union.DeepNoDive addrspace(200)*
  // CHECK-NEXT: [[TMP16:%[0-9]+]] = load %union.DeepNoDive, %union.DeepNoDive addrspace(200)* [[TMP15]], align [[$CAP_SIZE]]
  // CHECK-NEXT: store %union.DeepNoDive [[TMP16]], %union.DeepNoDive addrspace(200)* [[TMP14]], align [[$CAP_SIZE]]
}

void test_twolevels_nodive() {
  union DeepNoDive d;
  d.A.B.str = "hello";
  // CHECK: [[TMP17:%[0-9]+]] = bitcast %union.DeepNoDive addrspace(200)* {{.*}} to i8 addrspace(200)*
  // CHECK-NEXT: call void @test_twolevels_nodive_helper(i8 addrspace(200)* inreg [[TMP17]])
  test_twolevels_nodive_helper(d);
}
