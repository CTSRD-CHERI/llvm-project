; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'builtin.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

@results = common global [12 x i8 addrspace(200)*] zeroinitializer, align 32

; CHECK: test
; Function Attrs: nounwind
define i64 @test(i8 addrspace(200)* %rfoo) #0 {
  %r1 = alloca i8 addrspace(200)*, align 32
  %rx = alloca i64, align 8
  store i8 addrspace(200)* %rfoo, i8 addrspace(200)** %r1, align 32
  %r2 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgetlen
  %r3 = call i64 @llvm.cheri.cap.length.get(i8 addrspace(200)* %r2)
  %r4 = load i64, i64* %rx, align 8
  %r5 = and i64 %r4, %r3
  store i64 %r5, i64* %rx, align 8
  %r6 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgetperm
  %r7 = call i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)* %r6)
  %r8 = trunc i64 %r7 to i16
  %r9 = sext i16 %r8 to i64
  %r10 = load i64, i64* %rx, align 8
  %r11 = and i64 %r10, %r9
  store i64 %r11, i64* %rx, align 8
  %r12 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgettype
  %r13 = call i64 @llvm.cheri.cap.type.get(i8 addrspace(200)* %r12)
  %r14 = load i64, i64* %rx, align 8
  %r15 = and i64 %r14, %r13
  store i64 %r15, i64* %rx, align 8
  %r16 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: cgettag
  %r17 = call i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %r16)
  %r18 = trunc i64 %r17 to i1
  %r19 = zext i1 %r18 to i64
  %r20 = load i64, i64* %rx, align 8
  %r21 = and i64 %r20, %r19
  store i64 %r21, i64* %rx, align 8
  %r22 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  %r24 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
  ; CHECK: candperm
  %r25 = call i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)* %r24, i64 12)
  store i8 addrspace(200)* %r25, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*], [12 x i8 addrspace(200)*]* @results, i32 0, i64 1), align 32
  %r26 = load i8 addrspace(200)*, i8 addrspace(200)** %r1, align 32
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
  %r39 = call i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* %r16)
  %r40 = trunc i64 %r39 to i1
  %r41 = zext i1 %r40 to i64
  %r42 = and i64 %r41, %r38
  ; CHECK: ccheckperm
  call void @llvm.cheri.cap.perms.check(i8 addrspace(200)* %r30, i64 12)
  ; CHECK: cchecktype
  call void @llvm.cheri.cap.type.check(i8 addrspace(200)* %r30, i8 addrspace(200)* %r31)
  ; CHECK: jr
  ret i64 %r42
}

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.length.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.type.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.length.set(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.type.set(i8 addrspace(200)*, i64) #1


; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare void @llvm.cheri.cap.perms.check(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare void @llvm.cheri.cap.type.check(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.mips.cap.cause.set(i64) #2

; Function Attrs: nounwind
declare i64 @llvm.mips.cap.cause.get() #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.4 "}
