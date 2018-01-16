; RUN: %cheri_purecap_llc -o - %s -O0 | FileCheck %s -check-prefixes CHECK
; RUN: %cheri_purecap_llc -o - %s -O0 -filetype=obj | llvm-objdump -r -d - | FileCheck %s -check-prefix OBJ
; RUN: %cheri128_purecap_llc -o - %s | FileCheck %s -check-prefixes ASM,ASM128
; RUN: %cheri256_purecap_llc -o - %s | FileCheck %s -check-prefixes ASM,ASM256

; FIXME: Why is LLVM generating an invalid cgetpccsetoffset
; CHECK-NOT: cgetpccsetoffset        $c12, test
; OBJ-NOT:   cgetpccsetoffset        $c12, $zero
; llc without relocation-model pic generates invalid code
; XFAIL: *
; See clang/test/CodeGen/cheri-vla.c
source_filename = "cheri-vla.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: noinline nounwind optnone
define void @foo() #0 {
entry:
  %i = alloca i64, align 8, addrspace(200)
  %saved_stack = alloca i8 addrspace(200)*, align 32, addrspace(200)
  store i64 1, i64 addrspace(200)* %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i64, i64 addrspace(200)* %i, align 8
  %cmp = icmp slt i64 %0, 32
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i64, i64 addrspace(200)* %i, align 8
  %2 = call i8 addrspace(200)* @llvm.stacksave.p200i8()
  store i8 addrspace(200)* %2, i8 addrspace(200)* addrspace(200)* %saved_stack, align 32
  %vla = alloca i8, i64 %1, align 1, addrspace(200)
  %3 = load i64, i64 addrspace(200)* %i, align 8
  %sub = sub nsw i64 %3, 1
  %arrayidx = getelementptr inbounds i8, i8 addrspace(200)* %vla, i64 %sub
  store i8 0, i8 addrspace(200)* %arrayidx, align 1
  call void @test(i8 addrspace(200)* %vla)
  %4 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %saved_stack, align 32
  call void @llvm.stackrestore.p200i8(i8 addrspace(200)* %4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i64, i64 addrspace(200)* %i, align 8
  %inc = add nsw i64 %5, 1
  store i64 %inc, i64 addrspace(200)* %i, align 8
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; ASM-DAG: daddiu  [[LOOP_REG:\$[0-9]+]], $zero, 1
; ASM-DAG: ld [[TEST_ADDR:\$[0-9]+]], %call16(test)($gp)
; ASM-DAG: daddiu  [[COUNT_REG:\$[0-9]+]], $zero, 32
; ASM: .LBB0_1:
; ASM: cmove     [[SAVEREG:\$c[0-9]+]], $c11
; ASM256: daddiu  $1, [[LOOP_REG]], 31
; ASM128: daddiu  $1, [[LOOP_REG]], 15
; ASM: and     $1, $1, $18
; ASM: dsubu   $1, $sp, $1
; ASM: move     $sp, $1
; ASM: csetoffset      $c1, $c11, $1
; ASM: csetbounds      $c3, $c1, [[LOOP_REG]]
; ASM: daddiu  $1, [[LOOP_REG]], -1
; ASM: cgetpccsetoffset        $c12, [[TEST_ADDR]]
; ASM: cjalr   $c12, $c17
; ASM: csb     $zero, $1, 0($c3)
; ASM: daddiu  [[LOOP_REG]], [[LOOP_REG]], 1
; ASM: bne     [[LOOP_REG]], [[COUNT_REG]], .LBB0_1
; ASM: move     $c11, [[SAVEREG]]


; Function Attrs: nounwind
declare i8 addrspace(200)* @llvm.stacksave.p200i8() #1

declare void @test(i8 addrspace(200)*) #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #3

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #3

; Function Attrs: nounwind
declare void @llvm.stackrestore.p200i8(i8 addrspace(200)*) #1

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 03021c0c5d80a6d29561e70bc814cc7f83cc9981) (https://github.com/llvm-mirror/llvm.git b443cde4265aeecd0ea4346fb3674b3f9ef2608b)"}
