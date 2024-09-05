; We would previously generate a broken CHERI capability load for a memcpy with dest aligned to cap size
; REQUIRES: mips-registered-target
; but src not aligned and then assert later on
; RUN: %cheri128_opt %s -passes=instcombine -S -o %t.ll
; RUN: FileCheck %s -input-file=%t.ll
; check that we don't generate broken output due to instcombine:
; RUN: %cheri128_llc -O2 %t.ll -o - | FileCheck %s -check-prefix ASM -implicit-check-not clc


%struct.anon = type { %struct.b }
%struct.b = type { %struct.anon.0 }
%struct.anon.0 = type { ptr addrspace(200) }

@g = common global %struct.anon zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone
define void @h() {
entry:
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 @g, ptr align 4 @h, i64 6, i1 false)
  ; This should not be turned into a load i8 addrspace(200)
  ; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(6) @g, ptr noundef nonnull align 4 dereferenceable(6) @h, i64 6, i1 false)
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)


; ASM: lw	$3, %lo(h)($2)
; ASM: lh	$2, 4($2)
; ASM: sw	$3, %lo(g)($1)
; ASM: $2, 4($1)
