// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm  -o - -O2 | FileCheck %s

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
  // CHECK: [[VAR0:%.+]] = call i8 addrspace(200)* @llvm.stacksave.p200i8()
  // CHECK: [[VLA:%.+]] = alloca i8, i64 [[I_05]], align 1
  // CHECK: [[SUB:%.+]] = add nsw i64 [[I_05]], -1
  // CHECK: [[ARRAYIDX:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[VLA]], i64 [[SUB]]
  // CHECK: store i8 0, i8 addrspace(200)* [[ARRAYIDX]], align 1, !tbaa !{{[0-9]+}}
  // CHECK: call void @test(i8 addrspace(200)* nonnull [[VLA]]) #1
  // CHECK: call void @llvm.stackrestore.p200i8(i8 addrspace(200)* [[VAR0]])
  // CHECK: [[INC]] = add nuw nsw i64 [[I_05]], 1
  // CHECK: [[EXITCOND:%.+]] = icmp eq i64 [[INC]], 32
  // CHECK: br i1 [[EXITCOND]], label %[[FOR_COND_CLEANUP:.+]], label %[[FOR_BODY]]
}
