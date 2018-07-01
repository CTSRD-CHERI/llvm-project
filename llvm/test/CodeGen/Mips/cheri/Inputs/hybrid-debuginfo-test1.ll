source_filename = "/src/test/CodeGen/cheri/cheri-debug-info.c"
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: noinline nounwind optnone
define i32 @foo(i32* %i) #0 !dbg !9 {
entry:
  %i.addr = alloca i32*, align 16
  %j = alloca i32, align 4
  %j1 = alloca i32, align 4
  store i32* %i, i32** %i.addr, align 16
  call void @llvm.dbg.declare(metadata i32** %i.addr, metadata !14, metadata !DIExpression()), !dbg !15
  %0 = load i32*, i32** %i.addr, align 16, !dbg !16
  %tobool = icmp ne i32* %0, null, !dbg !16
  br i1 %tobool, label %if.then, label %if.else, !dbg !18

if.then:                                          ; preds = %entry
  call void @llvm.dbg.declare(metadata i32* %j, metadata !19, metadata !DIExpression()), !dbg !21
  store i32 2, i32* %j, align 4, !dbg !21
  br label %if.end, !dbg !22

if.else:                                          ; preds = %entry
  call void @llvm.dbg.declare(metadata i32* %j1, metadata !23, metadata !DIExpression()), !dbg !25
  store i32 3, i32* %j1, align 4, !dbg !25
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load i32*, i32** %i.addr, align 16, !dbg !26
  %2 = bitcast i32* %1 to i8*, !dbg !27
  %3 = ptrtoint i8* %2 to i64, !dbg !27
  %4 = trunc i64 %3 to i32, !dbg !27
  ret i32 %4, !dbg !28
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (https://github.com/llvm-mirror/clang.git 3b79a88ce02dcb2e8ed16e813156f2d41cac7a83) (https://github.com/llvm-mirror/llvm.git f721a1b6115a98a0cde6f8714dd405b8273c34a9)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3)
!1 = !DIFile(filename: "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cheri/<stdin>", directory: "/Users/alex/cheri/ctsrd-svn/cheritest")
!2 = !{}
!3 = !{!4}
!4 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!5 = !{i32 2, !"Dwarf Version", i32 2}
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{!"clang version 7.0.0 (https://github.com/llvm-mirror/clang.git 3b79a88ce02dcb2e8ed16e813156f2d41cac7a83) (https://github.com/llvm-mirror/llvm.git f721a1b6115a98a0cde6f8714dd405b8273c34a9)"}
!9 = distinct !DISubprogram(name: "foo", scope: !10, file: !10, line: 20, type: !11, isLocal: false, isDefinition: true, scopeLine: 20, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
!10 = !DIFile(filename: "/src/test/CodeGen/cheri/cheri-debug-info.c", directory: "/src")
!11 = !DISubroutineType(types: !12)
!12 = !{!4, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!14 = !DILocalVariable(name: "i", arg: 1, scope: !9, file: !10, line: 20, type: !13)
!15 = !DILocation(line: 20, column: 14, scope: !9)
!16 = !DILocation(line: 21, column: 6, scope: !17)
!17 = distinct !DILexicalBlock(scope: !9, file: !10, line: 21, column: 6)
!18 = !DILocation(line: 21, column: 6, scope: !9)
!19 = !DILocalVariable(name: "j", scope: !20, file: !10, line: 22, type: !4)
!20 = distinct !DILexicalBlock(scope: !17, file: !10, line: 21, column: 9)
!21 = !DILocation(line: 22, column: 7, scope: !20)
!22 = !DILocation(line: 23, column: 2, scope: !20)
!23 = !DILocalVariable(name: "j", scope: !24, file: !10, line: 25, type: !4)
!24 = distinct !DILexicalBlock(scope: !17, file: !10, line: 24, column: 7)
!25 = !DILocation(line: 25, column: 7, scope: !24)
!26 = !DILocation(line: 27, column: 14, scope: !9)
!27 = !DILocation(line: 27, column: 9, scope: !9)
!28 = !DILocation(line: 27, column: 2, scope: !9)
