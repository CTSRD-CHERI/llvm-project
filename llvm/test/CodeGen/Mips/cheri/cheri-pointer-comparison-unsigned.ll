; check that pointer comparisons are unsigned:
; NGINX has a loop with (void*)-1 as a sentinel value which would previously never be entered

; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@str = private unnamed_addr constant [5 x i8] c"nope\00"
@str.2 = private unnamed_addr constant [3 x i8] c"ok\00"


define i32 @lt(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) local_unnamed_addr #0 {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
; CHECK: cltu	$2, $c3, $c4
}


; SOURCE CODE:
; int
; main(void)
; {
;         void *a, *b;
;
;         a = (void *)0x12033091e;
;         b = (void *)0xffffffffffffffff;
;
;         if (a < b) {
;                 printf("ok\n");
;                 return (0);
;         }
;
;         printf("surprising result\n");
;         return (1);
; }

; Function Attrs: nounwind
define i32 @main() local_unnamed_addr #0 {
entry:
%0 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 4835182878)
%1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 -1)
%cmp = icmp ult i8 addrspace(200)* %0, %1
br i1 %cmp, label %if.then, label %if.end

; CHECK:      cgetnull $c1
; CHECK-NEXT: lui     $1, 1
; CHECK-NEXT: daddiu  $1, $1, 8243
; CHECK-NEXT: dsll    $1, $1, 16
; CHECK-NEXT: daddiu  $1, $1, 2334
; CHECK-NEXT: cincoffset      [[RANDOM_POINTER_VALUE_CAP:\$c.+]], $c1, $1
; CHECK-NEXT: cincoffset      [[UINT64_MAX_CAP:\$c.+]], $c1, -1
; If block should be entered if random_value < UINT64_MAX, i.e. skip if UINT64_MAX <= random_value
; CHECK-NEXT: cleu    $1, [[UINT64_MAX_CAP]], [[RANDOM_POINTER_VALUE_CAP]]
; CHECK-NEXT: bnez    $1, .LBB1_2

; CHECK: # %bb.1:  # %if.then
; CHECK: jal     puts
; CHECK: j       .LBB1_3
; CHECK: .LBB1_2:  # %if.end



if.then: ; preds = %entry
%puts5 = tail call i32 @puts(i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds ([3 x i8], [3 x i8]* @str.2, i64 0, i64 0) to i8 addrspace(200)*))
br label %cleanup

if.end: ; preds = %entry
%puts = tail call i32 @puts(i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds ([5 x i8], [5 x i8]* @str, i64 0, i64 0) to i8 addrspace(200)*))
br label %cleanup

cleanup: ; preds = %if.end, %if.then
%retval.0 = phi i32 [ 0, %if.then ], [ 1, %if.end ]
ret i32 %retval.0
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind
declare i32 @puts(i8 addrspace(200)* nocapture readonly) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (https://github.com/CTSRD-CHERI/clang.git 4cbc334a5bb1dacd45a08e249730120bec6fe766) (https://github.com/CTSRD-CHERI/llvm.git c7e6e470f3b77b6383a9854c63a3c0772ef340bb)"}
