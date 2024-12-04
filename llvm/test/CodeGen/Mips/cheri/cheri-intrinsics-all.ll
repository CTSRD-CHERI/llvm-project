; RUN: %cheri_llc -O0 %s -o - | FileCheck %s

@results = common global [12 x ptr addrspace(200)] zeroinitializer, align 32

define i64 @test(ptr addrspace(200) %rfoo)nounwind {
  %r1 = alloca ptr addrspace(200), align 32
  %rx = alloca i64, align 8
  store ptr addrspace(200) %rfoo, ptr %r1, align 32
  %r2 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cgetlen
  %r3 = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %r2)
  %r4 = load i64, ptr %rx, align 8
  %r5 = and i64 %r4, %r3
  store i64 %r5, ptr %rx, align 8
  %r6 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cgetperm
  %r7 = call i64 @llvm.cheri.cap.perms.get.i64(ptr addrspace(200) %r6)
  %r8 = trunc i64 %r7 to i16
  %r9 = sext i16 %r8 to i64
  %r10 = load i64, ptr %rx, align 8
  %r11 = and i64 %r10, %r9
  store i64 %r11, ptr %rx, align 8
  %rgetflags1 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cgetflags
  %rgetflags2 = call i64 @llvm.cheri.cap.flags.get.i64(ptr addrspace(200) %rgetflags1)
  %rgetflags3 = load i64, ptr %rx, align 8
  %rgetflags4 = and i64 %rgetflags3, %rgetflags2
  store i64 %rgetflags4, ptr %rx, align 8
  %r12 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cgettype
  %r13 = call i64 @llvm.cheri.cap.type.get.i64(ptr addrspace(200) %r12)
  %r14 = load i64, ptr %rx, align 8
  %r15 = and i64 %r14, %r13
  store i64 %r15, ptr %rx, align 8
  %r16 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cgettag
  %r18 = call i1 @llvm.cheri.cap.tag.get(ptr addrspace(200) %r16)
  %r19 = zext i1 %r18 to i64
  %r20 = load i64, ptr %rx, align 8
  %r21 = and i64 %r20, %r19
  store i64 %r21, ptr %rx, align 8
  %r22 = load ptr addrspace(200), ptr %r1, align 32
  %r24 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: candperm
  %r25 = call ptr addrspace(200) @llvm.cheri.cap.perms.and.i64(ptr addrspace(200) %r24, i64 12)
  store ptr addrspace(200) %r25, ptr getelementptr inbounds ([12 x ptr addrspace(200)], ptr @results, i32 0, i64 1), align 32
  %r26 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: csetflags
  %r27 = call ptr addrspace(200) @llvm.cheri.cap.flags.set.i64(ptr addrspace(200) %r26, i64 0)
  store ptr addrspace(200) %r27, ptr getelementptr inbounds ([12 x ptr addrspace(200)], ptr @results, i32 0, i64 2), align 32
  %r30 = load ptr addrspace(200), ptr %r1, align 32
  %r31 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cseal
  %r32 = call ptr addrspace(200) @llvm.cheri.cap.seal(ptr addrspace(200) %r30, ptr addrspace(200) %r31)
  store ptr addrspace(200) %r32, ptr getelementptr inbounds ([12 x ptr addrspace(200)], ptr @results, i32 0, i64 4), align 32
  %r33 = load ptr addrspace(200), ptr %r1, align 32
  %r34 = load ptr addrspace(200), ptr %r1, align 32
  ; CHECK: cunseal
  %r35 = call ptr addrspace(200) @llvm.cheri.cap.unseal(ptr addrspace(200) %r33, ptr addrspace(200) %r34)
  store ptr addrspace(200) %r35, ptr getelementptr inbounds ([12 x ptr addrspace(200)], ptr @results, i32 0, i64 5), align 32
  ; CHECK: csetcause
  call void @llvm.mips.cap.cause.set(i64 42)
  %r36 = load i64, ptr %rx, align 8
  ; CHECK: cgetcause
  %r37 = call i64 @llvm.mips.cap.cause.get()
  %r38 = and i64 %r36, %r37
  ; This comes later, but the instruction scheduler puts it in here to avoid a
  ; spill
  ; CHECK: cgetsealed
  %r40 = call i1 @llvm.cheri.cap.sealed.get(ptr addrspace(200) %r16)
  %r41 = zext i1 %r40 to i64
  %r42 = load ptr addrspace(200), ptr %r1, align 32
  %r43 = load ptr addrspace(200), ptr %r1, align 32
  %r44 = and i64 %r41, %r38
  ; CHECK: ctestsubset
  %r46 = call i1 @llvm.cheri.cap.subset.test(ptr addrspace(200) %r42, ptr addrspace(200) %r43)
  %r47 = zext i1 %r46 to i64
  %r48 = and i64 %r47, %r44
  ; CHECK: ccheckperm
  call void @llvm.cheri.cap.perms.check.i64(ptr addrspace(200) %r30, i64 12)
  ; CHECK: cchecktype
  call void @llvm.cheri.cap.type.check(ptr addrspace(200) %r30, ptr addrspace(200) %r31)
  ; CHECK: jr
  ret i64 %r48
}

declare i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200))

declare i64 @llvm.cheri.cap.perms.get.i64(ptr addrspace(200))

declare i64 @llvm.cheri.cap.flags.get.i64(ptr addrspace(200))

declare i64 @llvm.cheri.cap.type.get.i64(ptr addrspace(200))

declare i1 @llvm.cheri.cap.sealed.get(ptr addrspace(200))

declare i1 @llvm.cheri.cap.tag.get(ptr addrspace(200))

declare ptr addrspace(200) @llvm.cheri.cap.length.set(ptr addrspace(200), i64)

declare ptr addrspace(200) @llvm.cheri.cap.perms.and.i64(ptr addrspace(200), i64)

declare ptr addrspace(200) @llvm.cheri.cap.flags.set.i64(ptr addrspace(200), i64)

declare ptr addrspace(200) @llvm.cheri.cap.type.set(ptr addrspace(200), i64)


declare ptr addrspace(200) @llvm.cheri.cap.seal(ptr addrspace(200), ptr addrspace(200))

declare ptr addrspace(200) @llvm.cheri.cap.unseal(ptr addrspace(200), ptr addrspace(200))

declare void @llvm.cheri.cap.perms.check.i64(ptr addrspace(200), i64)

declare void @llvm.cheri.cap.type.check(ptr addrspace(200), ptr addrspace(200))

declare i1 @llvm.cheri.cap.subset.test(ptr addrspace(200), ptr addrspace(200))

; Function Attrs: nounwind
declare void @llvm.mips.cap.cause.set(i64)

; Function Attrs: nounwind
declare i64 @llvm.mips.cap.cause.get()


!llvm.ident = !{!0}

!0 = !{!"clang version 3.4 "}
