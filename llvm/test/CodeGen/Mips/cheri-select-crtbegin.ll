; RUN: %cheri_llc -relocation-model=pic %s -o -
; ModuleID = '/home/alr48/cheri/sources/cheribsd/lib/csu/cheri/crtbeginC.c'
source_filename = "/home/alr48/cheri/sources/cheribsd/lib/csu/cheri/crtbeginC.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.capreloc = type { i64, i64, i64, i64, i64 }

@__CTOR_LIST__ = internal addrspace(200) global [1 x i64] [i64 -1], section ".ctors", align 8
@__DTOR_LIST__ = internal addrspace(200) global [1 x i64] [i64 -1], section ".dtors", align 8
@__CTOR_END__ = external addrspace(200) global i64, align 8
@__start___cap_relocs = extern_weak addrspace(200) global %struct.capreloc, align 8
@__stop___cap_relocs = extern_weak addrspace(200) global %struct.capreloc, align 8
@_int = common addrspace(200) global i32 0, align 4
@__dso_handle = common local_unnamed_addr addrspace(200) global i8 addrspace(200)* null, align 32
@llvm.used = appending global [2 x i8*] [i8* addrspacecast (i8 addrspace(200)* bitcast ([1 x i64] addrspace(200)* @__CTOR_LIST__ to i8 addrspace(200)*) to i8*), i8* addrspacecast (i8 addrspace(200)* bitcast ([1 x i64] addrspace(200)* @__DTOR_LIST__ to i8 addrspace(200)*) to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
define void @crt_call_constructors() local_unnamed_addr #0 {
  br label %1

; <label>:1:                                      ; preds = %0, %9
  %2 = phi i64 addrspace(200)* [ getelementptr inbounds ([1 x i64], [1 x i64] addrspace(200)* @__CTOR_LIST__, i64 0, i64 0), %0 ], [ %10, %9 ]
  %3 = load i64, i64 addrspace(200)* %2, align 8, !tbaa !1
  %4 = icmp eq i64 %3, -1
  br i1 %4, label %9, label %5

; <label>:5:                                      ; preds = %1
  %6 = tail call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %7 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %6, i64 %3)
  %8 = bitcast i8 addrspace(200)* %7 to void () addrspace(200)*
  ; XXXAR: this also causes an error: error: '%8' defined with type 'void () addrspace(200)*'
  ; tail call void %8() #2
  br label %9

; <label>:9:                                      ; preds = %1, %5
  %10 = getelementptr inbounds i64, i64 addrspace(200)* %2, i64 1
  %11 = icmp eq i64 addrspace(200)* %10, @__CTOR_END__
  br i1 %11, label %12, label %1

; <label>:12:                                     ; preds = %9
  ret void
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #1

; Function Attrs: nounwind
define void @crt_init_globals() local_unnamed_addr #0 {
  %1 = tail call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %2 = tail call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %3 = tail call i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)* %1, i64 -3)
  %4 = tail call i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)* %2, i64 -41)
  br i1 icmp ult (%struct.capreloc addrspace(200)* @__start___cap_relocs, %struct.capreloc addrspace(200)* @__stop___cap_relocs), label %5, label %7

; <label>:5:                                      ; preds = %0
  br label %8

; <label>:6:                                      ; preds = %32
  br label %7

; <label>:7:                                      ; preds = %6, %0
  ret void

; <label>:8:                                      ; preds = %5, %32
  %9 = phi %struct.capreloc addrspace(200)* [ %37, %32 ], [ @__start___cap_relocs, %5 ]
  %10 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 0, i32 4
  %11 = load i64, i64 addrspace(200)* %10, align 8, !tbaa !5
  %12 = icmp slt i64 %11, 0
  %13 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 0, i32 0
  %14 = load i64, i64 addrspace(200)* %13, align 8, !tbaa !8
  %15 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %3, i64 %14)
  %16 = bitcast i8 addrspace(200)* %15 to i8 addrspace(200)* addrspace(200)*
  %17 = select i1 %12, i8 addrspace(200)* %4, i8 addrspace(200)* %3
  %18 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 0, i32 1
  %19 = load i64, i64 addrspace(200)* %18, align 8, !tbaa !9
  %20 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %17, i64 %19)
  %21 = icmp eq i64 %19, 314736
  br i1 %21, label %22, label %26

; <label>:22:                                     ; preds = %8
  %23 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %17, i64 %11)
  %24 = bitcast i8 addrspace(200)* %23 to i32 addrspace(200)*
  %25 = load i32, i32 addrspace(200)* %24, align 4, !tbaa !10
  store volatile i32 %25, i32 addrspace(200)* @_int, align 4, !tbaa !10
  br label %26

; <label>:26:                                     ; preds = %22, %8
  br i1 %12, label %32, label %27

; <label>:27:                                     ; preds = %26
  %28 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 0, i32 3
  %29 = load i64, i64 addrspace(200)* %28, align 8, !tbaa !12
  switch i64 %29, label %30 [
    i64 0, label %32
    i64 -1, label %32
  ]

; <label>:30:                                     ; preds = %27
  %31 = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)* %20, i64 %29)
  br label %32

; <label>:32:                                     ; preds = %27, %27, %30, %26
  %33 = phi i8 addrspace(200)* [ %20, %26 ], [ %31, %30 ], [ %20, %27 ], [ %20, %27 ]
  %34 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 0, i32 2
  %35 = load i64, i64 addrspace(200)* %34, align 8, !tbaa !13
  %36 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %33, i64 %35)
  store i8 addrspace(200)* %36, i8 addrspace(200)* addrspace(200)* %16, align 32, !tbaa !14
  %37 = getelementptr inbounds %struct.capreloc, %struct.capreloc addrspace(200)* %9, i64 1
  %38 = icmp ult %struct.capreloc addrspace(200)* %37, @__stop___cap_relocs
  br i1 %38, label %8, label %6
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.ddc.get() #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)*, i64) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 51751dcb922a975f65aa92a0e50144ed5518c80b) (https://github.com/CTSRD-CHERI/llvm.git 84186a2b5a2f78646d8255956b547a390fdc0a97)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !7, i64 32}
!6 = !{!"capreloc", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32}
!7 = !{!"long", !3, i64 0}
!8 = !{!6, !7, i64 0}
!9 = !{!6, !7, i64 8}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !3, i64 0}
!12 = !{!6, !7, i64 24}
!13 = !{!6, !7, i64 16}
!14 = !{!15, !15, i64 0}
!15 = !{!"any pointer", !3, i64 0}
