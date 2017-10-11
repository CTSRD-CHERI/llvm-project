// RUN: %clang_cc1 %s "-target-abi" "purecap" -emit-llvm -triple cheri-unknown-freebsd -o - -O2 | FileCheck %s
// RUN: %cheri256_cc1 %s "-target-abi" "purecap" -triple cheri-unknown-freebsd -o - -S -O2 | FileCheck %s -check-prefixes ASM,ASM256
// RUN: %cheri128_cc1 %s "-target-abi" "purecap" -triple cheri-unknown-freebsd -o - -S -O2 | FileCheck %s -check-prefixes ASM,ASM128

extern void test(const char*);

void foo(void) {
  for (long i = 1; i < 32; i++) {
    char vla[i];
    vla[i - 1] = '\0';
    test(vla);
  }
  // CHECK-LABEL: define void @foo()
  // CHECK: br label %[[FOR_BODY:.+]]
  // CHECK: [[FOR_BODY]]:
  // CHECK: [[I_05:%.+]] = phi i64 [ 1, %[[ENTRY:.+]] ], [ [[INC:%.+]], %[[FOR_BODY]] ]
  // CHECK: [[VAR0:%.+]] = call i8* @llvm.stacksave.p200i8()
  // CHECK: [[VLA:%.+]] = alloca i8, i64 [[I_05]], align 1
  // CHECK: [[SUB:%.+]] = add nsw i64 [[I_05]], -1
  // CHECK: [[ARRAYIDX:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[VLA]], i64 [[SUB]]
  // CHECK: store i8 0, i8 addrspace(200)* [[ARRAYIDX]], align 1, !tbaa !{{[0-9]+}}
  // CHECK: call void @test(i8 addrspace(200)* nonnull [[VLA]]) #1
  // CHECK: call void @llvm.stackrestore.p200i8(i8* [[VAR0]])
  // CHECK: [[INC]] = add nuw nsw i64 [[I_05]], 1
  // CHECK: [[EXITCOND:%.+]] = icmp eq i64 [[INC]], 32
  // CHECK: br i1 [[EXITCOND]], label %[[FOR_COND_CLEANUP:.+]], label %[[FOR_BODY]]

  // ASM-DAG: daddiu  [[LOOP_REG:\$[0-9]+]], $zero, 1
  // ASM-DAG: ld [[TEST_ADDR:\$[0-9]+]], %call16(test)($gp)
  // ASM-DAG: daddiu  [[COUNT_REG:\$[0-9]+]], $zero, 32
  // ASM: .LBB0_1:
  // ASM: move     [[SAVEREG:\$[0-9]+]], $sp
  // ASM256: daddiu  $1, [[LOOP_REG]], 31
  // ASM128: daddiu  $1, [[LOOP_REG]], 15
  // ASM: and     $1, $1, $18
  // ASM: dsubu   $1, $sp, $1
  // ASM: move     $sp, $1
  // ASM: csetoffset      $c1, $c11, $1
  // ASM: csetbounds      $c3, $c1, [[LOOP_REG]]
  // ASM: daddiu  $1, [[LOOP_REG]], -1
  // ASM: cgetpccsetoffset        $c12, [[TEST_ADDR]]
  // ASM: cjalr   $c12, $c17
  // ASM: csb     $zero, $1, 0($c3)
  // ASM: daddiu  [[LOOP_REG]], [[LOOP_REG]], 1
  // ASM: bne     [[LOOP_REG]], [[COUNT_REG]], .LBB0_1
  // ASM: move     $sp, [[SAVEREG]]
}
