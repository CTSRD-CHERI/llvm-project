; RUN: %cheri_llc -O0 %s -o - | FileCheck %s

@results = common global [12 x i8 addrspace(200)*] zeroinitializer, align 32

; CHECK: test
; Function Attrs: nounwind
define i64 @test(i8 addrspace(200)* %rfoo) #0 {
  %r1 = alloca i8 addrspace(200)*, align 32
  %rx = alloca i64, align 8
  store i8 addrspace(200)* %rfoo, i8 addrspace(200)** %r1, align 32
  %r2 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgetlen
  %r3 = call i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)* %r2)
  %r4 = load i64, i64* %rx, align 8
  %r5 = and i64 %r4, %r3
  store i64 %r5, i64* %rx, align 8
  %r6 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgetperm
  %r7 = call i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)* %r6)
  %r8 = trunc i64 %r7 to i16
  %r9 = sext i16 %r8 to i64
  %r10 = load i64, i64* %rx, align 8
  %r11 = and i64 %r10, %r9
  store i64 %r11, i64* %rx, align 8
  %rgetflags1 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgetflags
  %rgetflags2 = call i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)* %rgetflags1)
  %rgetflags3 = load i64, i64* %rx, align 8
  %rgetflags4 = and i64 %rgetflags3, %rgetflags2
  store i64 %rgetflags4, i64* %rx, align 8
  %r12 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgettype
  %r13 = call i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)* %r12)
  %r14 = load i64, i64* %rx, align 8
  %r15 = and i64 %r14, %r13
  store i64 %r15, i64* %rx, align 8
  %r16 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgettag
  %r18 = call i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %r16)
  %r19 = zext i1 %r18 to i64
  %r20 = load i64, i64* %rx, align 8
  %r21 = and i64 %r20, %r19
  store i64 %r21, i64* %rx, align 8
  %r22 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r24 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: candperm
  %r25 = call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)* %r24, i64 12)
  store i8 addrspace(200)* %r25, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*], [12 x i8 addrspace(200)*]* @results, i32 0, i64 1), align 32
  %r26 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: csetflags
  %r27 = call i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)* %r26, i64 0)
  store i8 addrspace(200)* %r27, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*], [12 x i8 addrspace(200)*]* @results, i32 0, i64 2), align 32
  %r30 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r31 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cseal
  %r32 = call i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)* %r30, i8 addrspace(200)* %r31)
  store i8 addrspace(200)* %r32, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*], [12 x i8 addrspace(200)*]* @results, i32 0, i64 4), align 32
  %r33 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r34 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cunseal
  %r35 = call i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)* %r33, i8 addrspace(200)* %r34)
  store i8 addrspace(200)* %r35, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*], [12 x i8 addrspace(200)*]* @results, i32 0, i64 5), align 32
  ; CHECK: csetcause
  call void @llvm.mips.cap.cause.set(i64 42)
  %r36 = load i64, i64* %rx, align 8
  ; CHECK: cgetcause
  %r37 = call i64 @llvm.mips.cap.cause.get()
  %r38 = and i64 %r36, %r37
  ; This comes later, but the instruction scheduler puts it in here to avoid a
  ; spill
  ; CHECK: cgetsealed
  %r40 = call i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* %r16)
  %r41 = zext i1 %r40 to i64
  %r42 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r43 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r44 = and i64 %r41, %r38
  ; CHECK: ctestsubset
  %r46 = call i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* %r42, i8 addrspace(200)* %r43)
  %r47 = zext i1 %r46 to i64
  %r48 = and i64 %r47, %r44
  ; CHECK: ccheckperm
  call void @llvm.cheri.cap.perms.check.i64(i8 addrspace(200)* %r30, i64 12)
  ; CHECK: cchecktype
  call void @llvm.cheri.cap.type.check(i8 addrspace(200)* %r30, i8 addrspace(200)* %r31)
  ; CHECK: jr
  ret i64 %r48
}

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.length.set(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.type.set(i8 addrspace(200)*, i64) #1


; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: hassideeffects nounwind readnone
declare void @llvm.cheri.cap.perms.check.i64(i8 addrspace(200)*, i64) #3

; Function Attrs: hassideeffects nounwind readnone
declare void @llvm.cheri.cap.type.check(i8 addrspace(200)*, i8 addrspace(200)*) #3

; Function Attrs: nounwind readnone
declare i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.mips.cap.cause.set(i64) #2

; Function Attrs: nounwind
declare i64 @llvm.mips.cap.cause.get() #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { hassideeffects nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.4 "}
