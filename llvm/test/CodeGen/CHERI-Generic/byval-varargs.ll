; Check that we can handle varargs functions with byval arguments
; First check that non-CHERI targets add a memcpy
; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | sed 's/p200i8/p0i8/g' > %t-nocheri.ll
; RUN: llc -mtriple riscv64-unknown-freebsd -o - -relocation-model=pic < %t-nocheri.ll | FileCheck %s --check-prefixes CHECK,RV64
; RUN: llc -mtriple mips64-unknown-freebsd -o - -relocation-model=pic  < %t-nocheri.ll | FileCheck %s --check-prefixes CHECK,MIPS
; Next check purecap targets:
; RUN: %riscv64_cheri_purecap_llc -o - %s | FileCheck %s --check-prefixes CHECK,PURECAP-RV64
; RUN: %cheri_purecap_llc -o - %s | FileCheck %s --check-prefixes CHECK,PURECAP-MIPS


; Note: This test case was generated from the following C
; int varargs(int, ...);
; struct Foo { char x[1024]; };
; int byref(struct Foo*);
; int test() {
;   struct Foo f;
;   __builtin_memset(&f, 0, sizeof(f));
;   byref(&f);
;   return varargs(sizeof(f), f);
; }
;
; The clang frontend uses byval arguments for MIPS and local allocas for RISC-V
; Check that we can handle both kinds of IR in either backend


%struct.Foo = type { [1024 x i8] }

; Function Attrs: nounwind
define signext i32 @test_alloca() local_unnamed_addr addrspace(200) #0 {
entry:
  %f = alloca %struct.Foo, align 1, addrspace(200)
  %byval-temp = alloca %struct.Foo, align 1, addrspace(200)
  %0 = getelementptr inbounds %struct.Foo, %struct.Foo addrspace(200)* %f, i64 0, i32 0, i64 0
  call void @llvm.lifetime.start.p200i8(i64 1024, i8 addrspace(200)* nonnull %0) #4
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* nonnull align 1 dereferenceable(1024) %0, i8 0, i64 1024, i1 false)
  %call = call signext i32 @byref(%struct.Foo addrspace(200)* nonnull %f) #4
  %1 = getelementptr inbounds %struct.Foo, %struct.Foo addrspace(200)* %byval-temp, i64 0, i32 0, i64 0
  call void @llvm.lifetime.start.p200i8(i64 1024, i8 addrspace(200)* nonnull %1) #4
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nonnull align 1 dereferenceable(1024) %1, i8 addrspace(200)* nonnull align 1 dereferenceable(1024) %0, i64 1024, i1 false)
  %call1 = call signext i32 (i32, ...) @varargs(i32 signext 1024, %struct.Foo addrspace(200)* nonnull %byval-temp) #4
  call void @llvm.lifetime.end.p200i8(i64 1024, i8 addrspace(200)* nonnull %1) #4
  call void @llvm.lifetime.end.p200i8(i64 1024, i8 addrspace(200)* nonnull %0) #4
  ret i32 %call1
}

; CHECK-LABEL: test_alloca:
; RV64:      addi [[LOCAL_VAR:s0]], sp, 1032
; RV64:      addi a2, zero, 1024
; RV64:      call memset@plt
; RV64-NEXT: mv a0, [[LOCAL_VAR]]
; RV64-NEXT: call byref@plt
; RV64:      addi [[BYVAL_COPY:s1]], sp, 8
; RV64:      addi a2, zero, 1024
; memcpy for local alloca: dst=BYVAL_COPY, src=LOCAL_VAR, size=1024
; RV64-NEXT: mv a0, [[BYVAL_COPY]]
; RV64-NEXT: mv a1, [[LOCAL_VAR]]
; RV64-NEXT: call memcpy@plt
; RV64-NEXT: addi a0, zero, 1024
; RV64-NEXT: mv a1, [[BYVAL_COPY]]
; RV64-NEXT: call varargs@plt

; PURECAP-RV64:       addi [[REG_1024:s1]], zero, 1024
; PURECAP-RV64:       csetbounds [[LOCAL_VAR:cs0]], {{.+}}, [[REG_1024]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(memset)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64:      addi a2, zero, 1024
; PURECAP-RV64-NEXT: cmove ca0, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: mv a1, zero
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(byref)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64-NEXT: cmove ca0, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; Note: memcpy for local alloca: dst=BYVAL_COPY, src=LOCAL_VAR, size=1024
; PURECAP-RV64:      cincoffset [[TMP:ca0]], csp, 32
; PURECAP-RV64-NEXT: csetbounds [[BYVAL_COPY:cs1]], [[TMP]], [[REG_1024]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(memcpy)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64-NEXT: addi a2, zero, 1024
; PURECAP-RV64-NEXT: cmove ca0, [[BYVAL_COPY]]
; PURECAP-RV64-NEXT: cmove ca1, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(varargs)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64-NEXT: addi a0, zero, 1024
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]


; MIPS:      daddiu [[LOCAL_VAR:\$16]], $sp, 1024
; MIPS:      ld $25, %call16(memset)($gp)
; memset(LOCAL_VAR)
; MIPS:      move $4, [[LOCAL_VAR]]
; MIPS:      jalr $25
; MIPS-NEXT: daddiu $6, $zero, 1024
; MIPS:      ld $25, %call16(byref)($gp)
; MIPS:      jalr $25
; MIPS-NEXT: move $4, $16
; MIPS:      daddiu [[BYVAL_COPY:\$17]], $sp, 0
; MIPS:      ld $25, %call16(memcpy)($gp)
; memcpy for local alloca: dst=BYVAL_COPY, src=LOCAL_VAR, size=1024
; MIPS:      move $4, [[BYVAL_COPY]]
; MIPS:      move $5, [[LOCAL_VAR]]
; MIPS:      jalr $25
; MIPS-NEXT: daddiu $6, $zero, 1024
; varargs() should use the byval copy:
; MIPS:      ld $25, %call16(varargs)($gp)
; MIPS:      daddiu $4, $zero, 1024
; MIPS:      jalr $25
; MIPS-NEXT: move $5, [[BYVAL_COPY]]

; PURECAP-MIPS: clcbi $c12, %capcall20(memset)($c18)
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1056
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c3]], $c11, $1
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 0
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: daddiu $5, $zero, 1024
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(byref)($c18)
; Note: we rematerialize the bounded var here to avoid spilling to the stack:
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1056
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c3]], $c11, $1
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: cincoffset [[BYVAL_COPY1:\$c19]], $c11, 32
; PURECAP-MIPS-NEXT: csetbounds [[BYVAL_COPY1]], [[BYVAL_COPY1]], 1024
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(memcpy)($c18)
; PURECAP-MIPS-NEXT: cincoffset [[BYVAL_COPY2:\$c3]], $c11, 32
; PURECAP-MIPS-NEXT: csetbounds [[BYVAL_COPY2]], [[BYVAL_COPY2]], 1024
; Note: we rematerialize the bounded var here to avoid spilling to the stack:
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1056
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c4]], $c11, $1
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 1024
; Save bounded byval copy to the stack and indirectly pass a bounded cap in $c13:
; Note: should probably rematerialize here instead of spilling
; PURECAP-MIPS-NEXT: csc [[BYVAL_COPY1]], $zero, 0($c11)
; PURECAP-MIPS-NEXT: csetbounds $c1, $c11, 16
; PURECAP-MIPS-NEXT: ori $1, $zero, 65495
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(varargs)($c18)
; PURECAP-MIPS-NEXT: candperm $c13, $c1, $1
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 1024


; Function Attrs: nounwind
define signext i32 @test_byval() local_unnamed_addr addrspace(200) #0 {
entry:
  %f = alloca %struct.Foo, align 8, addrspace(200)
  %0 = getelementptr inbounds %struct.Foo, %struct.Foo addrspace(200)* %f, i64 0, i32 0, i64 0
  call void @llvm.lifetime.start.p200i8(i64 1024, i8 addrspace(200)* nonnull %0) #4
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* nonnull align 8 dereferenceable(1024) %0, i8 0, i64 1024, i1 false)
  %call = call signext i32 @byref(%struct.Foo addrspace(200)* nonnull %f) #4
  %call1 = call signext i32 (i32, ...) @varargs(i32 signext 1024, %struct.Foo addrspace(200)* nonnull byval(%struct.Foo) align 8 %f) #4
  call void @llvm.lifetime.end.p200i8(i64 1024, i8 addrspace(200)* nonnull %0) #4
  ret i32 %call1
}

; CHECK-LABEL: test_byval:
; Stack frame size should be > 2048 (for some reason it's split into two addi instructions)
; RV64:      addi sp, sp, -2032
; RV64:      addi sp, sp, -48
; RV64:      addi [[LOCAL_VAR:s0]], sp, 1032
; RV64:      addi a2, zero, 1024
; RV64:      call memset@plt
; RV64-NEXT: mv a0, [[LOCAL_VAR]]
; RV64-NEXT: call byref@plt
; RV64:      addi [[BYVAL_COPY:s1]], sp, 8
; RV64:      addi a2, zero, 1024
; memcpy for local alloca: dst=BYVAL_COPY, src=LOCAL_VAR, size=1024
; RV64-NEXT: mv a0, [[BYVAL_COPY]]
; RV64-NEXT: mv a1, [[LOCAL_VAR]]
; RV64-NEXT: call memcpy@plt
; RV64-NEXT: addi a0, zero, 1024
; RV64-NEXT: mv a1, [[BYVAL_COPY]]
; RV64-NEXT: call varargs@plt

; Stack frame size should be > 2048 (for some reason it's split into two cincoffset instructions)
; PURECAP-RV64:      cincoffset csp, csp, -2032
; PURECAP-RV64:      cincoffset csp, csp, -96
; PURECAP-RV64:      addi [[REG_1024:a0]], zero, 1024
; PURECAP-RV64:      csetbounds [[LOCAL_VAR:cs0]], {{.+}}, [[REG_1024]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(memset)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64:      addi a2, zero, 1024
; PURECAP-RV64-NEXT: cmove ca0, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: mv a1, zero
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(byref)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64-NEXT: cmove ca0, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; Note: memcpy for local alloca: dst=BYVAL_COPY, src=LOCAL_VAR, size=1024
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(memcpy)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; Note: no bounds for implicit byval arg memcpy()
; TODO: should we add the csetbounds here? Not really necessary if we trust memcpy().
; PURECAP-RV64-NEXT: cincoffset [[BYVAL_COPY:cs1]], csp, 32
; PURECAP-RV64-NEXT: addi a2, zero, 1024
; PURECAP-RV64-NEXT: cmove ca0, [[BYVAL_COPY]]
; PURECAP-RV64-NEXT: cmove ca1, [[LOCAL_VAR]]
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]
; PURECAP-RV64:      auipcc [[JMP_TARGET:c[a-z0-9]+]], %captab_pcrel_hi(varargs)
; PURECAP-RV64-NEXT: clc [[JMP_TARGET]], %pcrel_lo(.LBB{{.+}})([[JMP_TARGET]])
; PURECAP-RV64-NEXT: addi a0, zero, 1024
; PURECAP-RV64-NEXT: cjalr [[JMP_TARGET]]


; Note: MIPS n64 passes up to the first 64 bytes of byval arguments in registers:
; Stack frame size should be just under 2048:
; MIPS:      daddiu $sp, $sp, -2032
; MIPS:      daddiu [[LOCAL_VAR:\$16]], $sp, 984
; MIPS:      ld $25, %call16(memset)($gp)
; memset(LOCAL_VAR)
; MIPS:      move $4, [[LOCAL_VAR]]
; MIPS:      jalr $25
; MIPS-NEXT: daddiu $6, $zero, 1024
; MIPS:      ld $25, %call16(byref)($gp)
; MIPS:      jalr $25
; MIPS-NEXT: move $4, [[LOCAL_VAR]]
; memcpy for local alloca: dst=BYVAL_COPY[56..1024]=$sp, src=LOCAL_VAR+56, size=1024-56=968
; MIPS:      daddiu $5, [[LOCAL_VAR]], 56
; MIPS:      ld $25, %call16(memcpy)($gp)
; MIPS:      move $4, $sp
; MIPS:      jalr $25
; MIPS-NEXT: daddiu $6, $zero, 968
; varargs() should use the byval copy:
; MIPS-NEXT: ld $11, 1032($sp)
; MIPS-NEXT: ld $10, 1024($sp)
; MIPS-NEXT: ld $9, 1016($sp)
; MIPS-NEXT: ld $8, 1008($sp)
; MIPS-NEXT: ld $7, 1000($sp)
; MIPS-NEXT: ld $6, 992($sp)
; MIPS-NEXT: ld $5, 984($sp)
; MIPS-NEXT: ld $25, %call16(varargs)($gp)
; MIPS:      jalr $25
; MIPS:      daddiu $4, $zero, 1024
; Note: rest of arguments on stack

; Stack frame size should be > 2048:
; PURECAP-MIPS:      daddiu $1, $zero, -2112
; PURECAP-MIPS-NEXT: cincoffset $c11, $c11, $1
; PURECAP-MIPS: clcbi $c12, %capcall20(memset)($c19)
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1040
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c3]], $c11, $1
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 0
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: daddiu $5, $zero, 1024
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(byref)($c19)
; Note: we rematerialize the bounded var here to avoid spilling to the stack:
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1040
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c3]], $c11, $1
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(memcpy)($c19)
; Note: we rematerialize the bounded var here to avoid spilling to the stack:
; PURECAP-MIPS-NEXT: daddiu $1, $zero, 1040
; PURECAP-MIPS-NEXT: cincoffset [[LOCAL_VAR:\$c4]], $c11, $1
; PURECAP-MIPS-NEXT: csetbounds [[LOCAL_VAR]], [[LOCAL_VAR]], 1024
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 1024
; PURECAP-MIPS-NEXT: cmove [[BYVAL_COPY:\$c18]], $c11
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: cmove $c3, [[BYVAL_COPY]]
; Save bounded byval copy to the stack and directly pass a bounded cap (1024 bytes) in $c13:
; Note: should probably rematerialize here instead of spilling
; PURECAP-MIPS-NEXT: csetbounds $c1, [[BYVAL_COPY]], 1024
; PURECAP-MIPS-NEXT: ori $1, $zero, 65495
; PURECAP-MIPS-NEXT: clcbi $c12, %capcall20(varargs)($c19)
; PURECAP-MIPS-NEXT: candperm $c13, $c1, $1
; PURECAP-MIPS-NEXT: cjalr $c12, $c17
; PURECAP-MIPS-NEXT: daddiu $4, $zero, 1024

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #1

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1 immarg) addrspace(200) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noalias nocapture writeonly, i8 addrspace(200)* noalias nocapture readonly, i64, i1 immarg) addrspace(200) #1

declare signext i32 @byref(%struct.Foo addrspace(200)*) local_unnamed_addr addrspace(200) #3

declare signext i32 @varargs(i32 signext, ...) local_unnamed_addr addrspace(200) #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #1

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { argmemonly nounwind willreturn writeonly }
attributes #3 = { nounwind }
attributes #4 = { nounwind }
