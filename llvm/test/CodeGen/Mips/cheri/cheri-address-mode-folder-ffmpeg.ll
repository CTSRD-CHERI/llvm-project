; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd -target-abi purecap -O3 -relocation-model=pic -verify-machineinstrs %s -o -
; ModuleID = '/Users/alex/cheri/llvm/test/CodeGen/Mips/cheri-address-mode-folder-ffmpeg.c'
; This test used to fail the machine verifier
; XFAIL: *
; The instruction after the hoisted global load kills the vreg but LLVM thinkgs it is needed after the basic block
source_filename = "/Users/alex/cheri/llvm/test/CodeGen/Mips/cheri-address-mode-folder-ffmpeg.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@c = common addrspace(200) global i32 addrspace(200)* null, align 32
@b = common addrspace(200) global i32 0, align 4
@a = common addrspace(200) global i32 0, align 4

; Function Attrs: noinline nounwind optnone
define i32 @fn1(i32 signext %p1) #0 {
entry:
  %retval = alloca i32, align 4, addrspace(200)
  %p1.addr = alloca i32, align 4, addrspace(200)
  %d = alloca i32, align 4, addrspace(200)
  store i32 %p1, i32 addrspace(200)* %p1.addr, align 4
  br label %while.cond

while.cond:                                       ; preds = %if.end7, %entry
  %0 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* @c, align 32
  %tobool = icmp ne i32 addrspace(200)* %0, null
  br i1 %tobool, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %1 = load i32, i32 addrspace(200)* %p1.addr, align 4
  %tobool1 = icmp ne i32 %1, 0
  br i1 %tobool1, label %land.lhs.true, label %if.end7

land.lhs.true:                                    ; preds = %while.body
  %2 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* @c, align 32
  %3 = load i32, i32 addrspace(200)* @b, align 4
  %conv = sext i32 %3 to i64
  %4 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* null, i64 %conv)
  %5 = bitcast i8 addrspace(200)* %4 to i32 addrspace(200)*
  %cmp = icmp ne i32 addrspace(200)* %2, %5
  br i1 %cmp, label %if.then, label %if.end7

if.then:                                          ; preds = %land.lhs.true
  %6 = load i32, i32 addrspace(200)* %p1.addr, align 4
  %and = and i32 %6, 1
  %tobool3 = icmp ne i32 %and, 0
  br i1 %tobool3, label %land.lhs.true4, label %if.end

land.lhs.true4:                                   ; preds = %if.then
  %call = call i32 (...) @fn2()
  %tobool5 = icmp ne i32 %call, 0
  br i1 %tobool5, label %if.then6, label %if.end

if.then6:                                         ; preds = %land.lhs.true4
  %7 = load i32, i32 addrspace(200)* %d, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32 addrspace(200)* %d, align 4
  store i32 %7, i32 addrspace(200)* @a, align 4
  br label %if.end

if.end:                                           ; preds = %if.then6, %land.lhs.true4, %if.then
  br label %if.end7

if.end7:                                          ; preds = %if.end, %land.lhs.true, %while.body
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %8 = load i32, i32 addrspace(200)* %retval, align 4
  ret i32 %8
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64) #1

declare i32 @fn2(...) #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #1

attributes #0 = { noinline nounwind optnone }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 1ea46f5291d82508565b82d4de939241775ab50c) (https://github.com/llvm-mirror/llvm.git f91be1b664a6070160972905806bf7c5ad3f7bee)"}
