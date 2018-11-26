; In order to reproduce the broken function crash we need to run the intrinsics
; folder more than once and also run other passes:
; RUN: %cheri_purecap_opt -S -cheri-fold-intrisics -O3 %s -o %t1.ll
; RUN: %cheri_purecap_opt -S -cheri-fold-intrisics -O3 %t1.ll -o /dev/null
; RUN: %cheri_purecap_llc -O2 %s -o /dev/null
; RUN: %cheri_purecap_llc -O2 %t1.ll -o /dev/null
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@d = common addrspace(200) global i32 0, align 4
@e = common addrspace(200) global i8 addrspace(200)* null, align 16

; C Source code:
;int d;
;void* e;
;void g(int x, int y) {
;  e = (__uintcap_t)&d + x + y;
;}

; O0 output :

; Function Attrs: noinline nounwind optnone
;define void @g(i32 signext %x, i32 signext %y) #0 {
;entry:
;  %x.addr = alloca i32, align 4, addrspace(200)
;  %y.addr = alloca i32, align 4, addrspace(200)
;  store i32 %x, i32 addrspace(200)* %x.addr, align 4
;  store i32 %y, i32 addrspace(200)* %y.addr, align 4
;  %0 = load i32, i32 addrspace(200)* %x.addr, align 4
;  %1 = sext i32 %0 to i64
;  %2 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 %1)
;  %3 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*))
;  %4 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %2)
;  %add = add i64 %3, %4
;  %5 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*), i64 %add)
;  %6 = load i32, i32 addrspace(200)* %y.addr, align 4
;  %7 = sext i32 %6 to i64
;  %8 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 %7)
;  %9 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %5)
;  %10 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %8)
;  %add1 = add i64 %9, %10
;  %11 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %5, i64 %add1)
;  store i8 addrspace(200)* %11, i8 addrspace(200)* addrspace(200)* @e, align 16
;  ret void
;}

; After one opt run:

; Function Attrs: nounwind
;define void @g(i32 signext %x, i32 signext %y) local_unnamed_addr #0 {
;entry:
;  %0 = sext i32 %x to i64
;  %1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 %0)
;  %2 = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*))
;  %add = add i64 %0, %2
;  %3 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*), i64 %0)
;  %4 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*), i64 %add)
;  %5 = sext i32 %y to i64
;  %6 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 %5)
;  %7 = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %3)
;  %add1 = add i64 %5, %7
;  %8 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %3, i64 %5)
;  %9 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %3, i64 %add1)
;  store i8 addrspace(200)* %8, i8 addrspace(200)* addrspace(200)* @e, align 16
;  ret void
;}

; After another run:
; Function Attrs: nounwind
define void @g(i32 signext %x, i32 signext %y) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %x to i64
  %1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* bitcast (i32 addrspace(200)* @d to i8 addrspace(200)*), i64 %0)
  %2 = sext i32 %y to i64
  %3 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %1, i64 %2)
  store i8 addrspace(200)* %3, i8 addrspace(200)* addrspace(200)* @e, align 16
  ret void
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*) #1

;; Function Attrs: nounwind readnone
;define i8 addrspace(200)* @fold_gep_incoffset(i8 addrspace(200)* %arg) local_unnamed_addr #1 {
;entry:
;  ; CHECK-LABEL: @fold_gep_incoffset()
;  ; CHECK: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %arg, i64 96)
;  %gep = getelementptr inbounds i8, i8 addrspace(200)* %arg, i64 -4
;  %inc = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %gep, i64 100)
;  ret i8 addrspace(200)* %inc
;}
;
;; Order of GEP vs incoffset should not matter:
;define i8 addrspace(200)* @fold_gep_incoffset2(i8 addrspace(200)* %arg) local_unnamed_addr #1 {
;entry:
;  ; CHECK-LABEL: @fold_gep_incoffset2()
;  ; CHECK: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %arg, i64 96)
;  %inc = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %arg, i64 100)
;  %gep = getelementptr inbounds i8, i8 addrspace(200)* %arg, i64 -4
;  ret i8 addrspace(200)* %gep
;}



attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
