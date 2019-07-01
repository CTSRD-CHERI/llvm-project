// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 %s -O0 -emit-llvm -o - | %cheri_FileCheck %s
// RUN: %cheri_purecap_cc1 %s -O2 -S -o - -mllvm -cheri-cap-table-abi=plt -mllvm -cheri-stack-bounds=if-needed | %cheri_FileCheck %s -check-prefixes ASM,%cheri_type

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
  // CHECK-LABEL: @test_onelevel_helper(
  // CHECK: [[TMP0:%[a-zA-Z0-9]+]] = alloca %union.CallData, align [[#CAP_SIZE]], addrspace(200)
  // CHECK-NEXT: [[TMP1:%[0-9]]] = bitcast i8 addrspace(200)* {{.*}}.coerce to %union.CallData addrspace(200)*
  // CHECK-NEXT: [[TMP2:%[0-9]]] = load %union.CallData, %union.CallData addrspace(200)* [[TMP1]], align [[#CAP_SIZE]]
  // CHECK-NEXT: store %union.CallData [[TMP2]], %union.CallData addrspace(200)* [[TMP0]], align [[#CAP_SIZE]]
}

void test_onelevel() {
  // CHECK-LABEL: @test_onelevel(
  union CallData foo;
  foo.Bar.c = "hello";
  foo.Bar.i = 9;
  // CHECK: [[TMP3:%[0-9]]] = bitcast %union.CallData addrspace(200)* {{.*}} to i8 addrspace(200)*
  // CHECK-NEXT: call void @test_onelevel_helper(i8 addrspace(200)* inreg [[TMP3]])
  test_onelevel_helper(foo);
}

void test_zerolevels_helper(union semun s) {
  // CHECK-LABEL: @test_zerolevels_helper(
  // CHECK: [[TMP4:%[a-zA-Z.]+]] = getelementptr inbounds %union.semun, %union.semun addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: store i8 addrspace(200)* {{.*}}, i8 addrspace(200)* addrspace(200)* [[TMP4]], align [[#CAP_SIZE]]
}

void test_zerolevels() {
  // CHECK-LABEL: @test_zerolevels(
  union semun s;
  s.buf1 = "hello";
  // CHECK: [[TMP5:%[a-zA-Z.]+]] = getelementptr inbounds %union.semun, %union.semun addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP6:%[0-9]+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[TMP5]], align [[#CAP_SIZE]] 
  // CHECK-NEXT: call void @test_zerolevels_helper(i8 addrspace(200)* inreg [[TMP6]])
  test_zerolevels_helper(s);
}

void test_twolevels_dive_helper(union DeepDive d) {
  // CHECK-LABEL: @test_twolevels_dive_helper(

  // CHECK: [[TMP7:%[a-zA-Z0-9.]+]] = getelementptr inbounds %union.DeepDive, %union.DeepDive addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP8:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.0, %struct.anon.0 addrspace(200)* [[TMP7]], i32 0, i32 0
  // CHECK-NEXT: [[TMP9:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.1, %struct.anon.1 addrspace(200)* [[TMP8]], i32 0, i32 0
  // CHECK-NEXT: store i8 addrspace(200)* {{.*}}, i8 addrspace(200)* addrspace(200)* [[TMP9]], align [[#CAP_SIZE]]
}

void test_twolevels_dive() {
  // CHECK-LABEL: @test_twolevels_dive(
  union DeepDive d;
  d.A.B.str = "hello";
  // CHECK: [[TMP10:%[a-zA-Z0-9.]+]] = getelementptr inbounds %union.DeepDive, %union.DeepDive addrspace(200)* {{.*}}, i32 0, i32 0
  // CHECK-NEXT: [[TMP11:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.0, %struct.anon.0 addrspace(200)* [[TMP10]], i32 0, i32 0
  // CHECK-NEXT: [[TMP12:%[a-zA-Z0-9.]+]] = getelementptr inbounds %struct.anon.1, %struct.anon.1 addrspace(200)* [[TMP11]], i32 0, i32 0
  // CHECK-NEXT: [[TMP13:%[0-9]+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[TMP12]], align [[#CAP_SIZE]] 
  // CHECK-NEXT: call void @test_twolevels_dive_helper(i8 addrspace(200)* inreg [[TMP13]])
  test_twolevels_dive_helper(d);
}

void test_twolevels_nodive_helper(union DeepNoDive d) {
  // CHECK-LABEL: @test_twolevels_nodive_helper(
  // CHECK: [[TMP14:%[a-zA-Z0-9]+]] = alloca %union.DeepNoDive, align [[#CAP_SIZE]], addrspace(200)
  // CHECK-NEXT: [[TMP15:%[0-9]+]] = bitcast i8 addrspace(200)* {{.*}} to %union.DeepNoDive addrspace(200)*
  // CHECK-NEXT: [[TMP16:%[0-9]+]] = load %union.DeepNoDive, %union.DeepNoDive addrspace(200)* [[TMP15]], align [[#CAP_SIZE]]
  // CHECK-NEXT: store %union.DeepNoDive [[TMP16]], %union.DeepNoDive addrspace(200)* [[TMP14]], align [[#CAP_SIZE]]
}

void test_twolevels_nodive() {
  union DeepNoDive d;
  d.A.B.str = "hello";
  // CHECK-LABEL: @test_twolevels_nodive(
  // CHECK: [[TMP17:%[0-9]+]] = bitcast %union.DeepNoDive addrspace(200)* {{.*}} to i8 addrspace(200)*
  // CHECK-NEXT: call void @test_twolevels_nodive_helper(i8 addrspace(200)* inreg [[TMP17]])
  test_twolevels_nodive_helper(d);
}

// Test that reading the values from semun works as expected
// Assembly tests shouldn't really be in clang but in this case we really want to verify that the calling convention matches

// For reading/writing the integer value we store the int and read back as capability/ write the cap and read back as an int
// TODO: could just use CIncOffset/CGetAddr here?

int test_semun_read_int(union semun s) {
  return s.val + 1;
  // ASM-LABEL: test_semun_read_int:
  // ASM:      cincoffset	$c11, $c11, -[[#CAP_SIZE]]
  // ASM-NEXT: csc	$c3, $zero, 0($c11)
  // ASM-NEXT: clw	$1, $zero, 0($c11)
  // ASM-NEXT: addiu	$2, $1, 1
  // ASM-NEXT: cjr	$c17
  // ASM: .end test_semun_read_int
}

extern int semun_pass_int_helper(union semun s);
int test_semun_pass_int(union semun s) {
  s.val = 1234;
  // ASM-LABEL: test_semun_pass_int:
  // ASM:      csc	$c3, $zero, 0($c11)
  // ASM-NEXT: addiu	$1, $zero, 1234
  // ASM-NEXT: csw	$1, $zero, 0($c11)
  // ASM:      clc	$c3, $zero, 0($c11)
  // ASM-LABEL: .end test_semun_pass_int
  return semun_pass_int_helper(s);
}

int test_semun_pass_int_2() {
  // ASM-LABEL: test_semun_pass_int_2:
  // CHERI256 does a memset to zero before (this happens because clang thinks it
  // should do it for aggregates > 16
  // TODO: do it for (> 16 || containsCaps)?
  // CHERI256:        csetbounds $c1, $c11, 32
  // CHERI256:        csc $cnull, $zero, 0($c1)
  // ASM:             addiu	$1, $zero, 1234
  // ASM-NEXT:        csw	$1, $zero, 0($c11)
  // ASM:             cjalr $c12, $c17
  // ASM-NEXT:        clc	$c3, $zero, 0($c11)
  // ASM-LABEL: .end test_semun_pass_int_2
  return semun_pass_int_helper((union semun){.val = 1234});
}

void* test_semun_read_cap(union semun s) {
  // ASM-LABEL: test_semun_read_cap:
  // ASM:      cjr $c17
  // ASM-NEXT: cincoffset	$c3, $c3, 1
  // ASM: .end test_semun_read_cap
  return s.buf1 + 1;
}
