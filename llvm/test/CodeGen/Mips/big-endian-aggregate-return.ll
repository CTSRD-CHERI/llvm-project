; RUN: opt -march=cheri -mcpu=mips64r2 -target-abi=n64 -relocation-model=pic -o - -filetype=asm -O3 -S < %s | FileCheck %s
; RUN: opt -march=mips64 -mcpu=cheri -target-abi=n64 -relocation-model=pic -o - -filetype=asm -O3 -S < %s | FileCheck %s
; XFAIL: *
; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/mips-big-endian-aggregate-return.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/mips-big-endian-aggregate-return.c"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd"

%struct.IntAndLong = type { i32, i64 }

@int_and_long.t = private unnamed_addr constant %struct.IntAndLong { i32 2, i64 3 }, align 8

; Function Attrs: noinline nounwind
define inreg { i64, i64 } @int_and_long() #0 {
entry:
  %0 = alloca %struct.IntAndLong, align 8
  %t = alloca %struct.IntAndLong, align 8
  %1 = bitcast %struct.IntAndLong* %t to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* bitcast (%struct.IntAndLong* @int_and_long.t to i8*), i64 16, i32 8, i1 false)
  %2 = bitcast %struct.IntAndLong* %0 to i8*
  %3 = bitcast %struct.IntAndLong* %t to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %2, i8* %3, i64 16, i32 8, i1 false)
  %4 = bitcast %struct.IntAndLong* %0 to { i64, i64 }*
  %5 = load { i64, i64 }, { i64, i64 }* %4, align 8
  ret { i64, i64 } %5
}

; CHECK-LABEL: define inreg { i64, i64 } @int_and_long()
; CHECK-NEXT: entry:
; CHECK-NEXT:   ret { i64, i64 } { i64 2, i64 3 }
; CHECK-NEXT: }



; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64r2" "target-features"="+mips64r2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 123c32250eaba71153a66e8a255a8b3e481f8a9c) (https://github.com/llvm-mirror/llvm.git a92312a679298db243d73c15b12e177cf43e377f)"}
