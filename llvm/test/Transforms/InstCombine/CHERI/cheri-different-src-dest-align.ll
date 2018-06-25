; We would previously generate a broken CHERI capability load for a memcpy with dest aligned to cap size
; but src not aligned and the assert later on
; RUN: %cheri128_opt %s -instcombine -S -o %t.ll
; RUN: cat %t.ll
; RUN: FileCheck %s -input-file=%t.ll
; check that we don't generate broken output due to instcombine:
; RUN: %cheri128_llc -O2 %t.ll -o - | FileCheck %s -check-prefix ASM -implicit-check-not clc


%struct.anon = type { %struct.b }
%struct.b = type { %struct.anon.0 }
%struct.anon.0 = type { i8 addrspace(200)* }

@g = common global %struct.anon zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone
define void @h() #0 {
entry:
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 bitcast (%struct.anon* @g to i8*), i8* align 4 bitcast (void ()* @h to i8*), i64 6, i1 false)
  ; This should not be turned into a load i8 addrspace(200)
  ; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 bitcast (%struct.anon* @g to i8*), i8* align 4 bitcast (void ()* @h to i8*), i64 6, i1 false)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #1


; ASM: lw	$3, %lo(h)($2)
; ASM: lh	$2, 4($2)
; ASM: sw	$3, %lo(g)($1)
; ASM: $2, 4($1)
