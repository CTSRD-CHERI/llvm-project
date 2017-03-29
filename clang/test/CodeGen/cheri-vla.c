// RUN: %clang_cc1 %s "-target-abi" "purecap" -triple cheri-unknown-freebsd -o - -S -O2 | FileCheck %s -check-prefix ASM
// RUN: %clang_cc1 %s "-target-abi" "purecap" -emit-llvm -triple cheri-unknown-freebsd -o - -O2 | FileCheck %s

extern void test(const char*);

void foo(void) {
  for (long i = 1; i < 32; i++) {
    char vla[i];
    vla[i - 1] = '\0';
    test(vla);
  }
  // CHECK-LABEL: for.body:
  // CHECK: %i.05 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  // CHECK: %0 = call i8* @llvm.stacksave()
  // CHECK: %vla = alloca i8, i64 %i.05, align 1
  // CHECK: %sub = add nsw i64 %i.05, -1
  // CHECK: %arrayidx = getelementptr inbounds i8, i8 addrspace(200)* %vla, i64 %sub
  // CHECK: store i8 0, i8 addrspace(200)* %arrayidx, align 1, !tbaa !1
  // CHECK: call void @test(i8 addrspace(200)* nonnull %vla) #1
  // CHECK: call void @llvm.stackrestore(i8* %0)
  // CHECK: %inc = add nuw nsw i64 %i.05, 1
  // CHECK: %exitcond = icmp eq i64 %inc, 32
  // CHECK: br i1 %exitcond, label %for.cond.cleanup, label %for.body

  // ASM: move     [[SAVEREG:\$[0-9]+]], $sp
  // ASM: daddiu  $1, $17, 31
  // ASM: and     $1, $1, $18
  // ASM: dsubu   $1, $sp, $1
  // ASM: move     $sp, $1
  // ASM: csetoffset      $c1, $c11, $1
  // ASM: csetbounds      $c3, $c1, $17
  // ASM: daddiu  $1, $17, -1
  // ASM: cgetpccsetoffset        $c12, $16
  // ASM: cjalr   $c12, $c17
  // ASM: csb     $zero, $1, 0($c3)
  // ASM: daddiu  $17, $17, 1
  // ASM: bne     $17, $19, .LBB0_1
  // ASM: move     $sp, [[SAVEREG]]
}
