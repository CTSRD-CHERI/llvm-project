; RUN: llc -mtriple mips64-unknown-freebsd12.0 -relocation-model pic -thread-model posix -mcpu=mips4 -target-abi n64 -O2 -o - %s
; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/update_editor-790bb6-reduce.test.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/update_editor-790bb6-reduce.test.c"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd12.0"
; https://bugs.llvm.org/show_bug.cgi?id=35071
; XFAIL: *

; Function Attrs: nounwind
define i32 @f() local_unnamed_addr #0 !dbg !8 {
entry:
  %g = alloca i32, align 4
  %call = tail call i32 bitcast (i32 (...)* @i to i32 ()*)() #4, !dbg !19
  %cmp = icmp eq i32 %call, 0, !dbg !20
  %conv = zext i1 %cmp to i32, !dbg !20
  tail call void @llvm.dbg.value(metadata i32 %conv, metadata !14, metadata !DIExpression()), !dbg !21
  %0 = bitcast i32* %g to i8*, !dbg !22
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #4, !dbg !22
  tail call void @llvm.dbg.value(metadata i32* %g, metadata !15, metadata !DIExpression()), !dbg !23
  %call1 = call i32 bitcast (i32 (...)* @h to i32 (i32*)*)(i32* nonnull %g) #4, !dbg !24
  call void @llvm.dbg.value(metadata i32 %call1, metadata !17, metadata !DIExpression()), !dbg !24
  %tobool = icmp eq i32 %call1, 0, !dbg !25
  br i1 %tobool, label %if.end, label %cleanup7.critedge, !dbg !24

if.end:                                           ; preds = %entry
  %1 = load i32, i32* %g, align 4, !dbg !27, !tbaa !29
  call void @llvm.dbg.value(metadata i32 %1, metadata !15, metadata !DIExpression()), !dbg !23
  %tobool2 = icmp eq i32 %1, 0, !dbg !27
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #4, !dbg !33
  br i1 %tobool2, label %cleanup.cont, label %cleanup7

cleanup.cont:                                     ; preds = %if.end
  %call6 = call i32 bitcast (i32 (...)* @j to i32 (i32)*)(i32 signext %conv) #4, !dbg !34
  br label %cleanup7, !dbg !35

cleanup7.critedge:                                ; preds = %entry
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #4, !dbg !33
  br label %cleanup7

cleanup7:                                         ; preds = %cleanup7.critedge, %if.end, %cleanup.cont
  ret i32 0, !dbg !35
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @i(...) local_unnamed_addr #2

declare i32 @h(...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

declare i32 @j(...) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips4" "target-features"="+mips4,+soft-float,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips4" "target-features"="+mips4,+soft-float,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 305865255b11af3a8dafef4aa679ea92a6370e93) (https://github.com/llvm-mirror/llvm.git 620602b1bc4cd0c8c92b60a32831f63cfbee6cdf)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/<stdin>", directory: "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 2}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 305865255b11af3a8dafef4aa679ea92a6370e93) (https://github.com/llvm-mirror/llvm.git 620602b1bc4cd0c8c92b60a32831f63cfbee6cdf)"}
!8 = distinct !DISubprogram(name: "f", scope: !9, file: !9, line: 8, type: !10, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: true, unit: !0, variables: !13)
!9 = !DIFile(filename: "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/update_editor-790bb6-reduce.test.c", directory: "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI")
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15, !17}
!14 = !DILocalVariable(name: "e", scope: !8, file: !9, line: 9, type: !12)
!15 = !DILocalVariable(name: "g", scope: !16, file: !9, line: 11, type: !12)
!16 = distinct !DILexicalBlock(scope: !8, file: !9, line: 10, column: 3)
!17 = !DILocalVariable(name: "d", scope: !16, file: !9, line: 12, type: !18)
!18 = !DIDerivedType(tag: DW_TAG_typedef, name: "a", file: !9, line: 2, baseType: !12)
!19 = !DILocation(line: 9, column: 11, scope: !8)
!20 = !DILocation(line: 9, column: 15, scope: !8)
!21 = !DILocation(line: 9, column: 7, scope: !8)
!22 = !DILocation(line: 11, column: 5, scope: !16)
!23 = !DILocation(line: 11, column: 9, scope: !16)
!24 = !DILocation(line: 12, column: 5, scope: !16)
!25 = !DILocation(line: 12, column: 5, scope: !26)
!26 = distinct !DILexicalBlock(scope: !16, file: !9, line: 12, column: 5)
!27 = !DILocation(line: 13, column: 9, scope: !28)
!28 = distinct !DILexicalBlock(scope: !16, file: !9, line: 13, column: 9)
!29 = !{!30, !30, i64 0}
!30 = !{!"int", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C/C++ TBAA"}
!33 = !DILocation(line: 15, column: 3, scope: !8)
!34 = !DILocation(line: 16, column: 3, scope: !8)
!35 = !DILocation(line: 17, column: 1, scope: !8)
