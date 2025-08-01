; RUN: opt -safe-stack -S -mtriple=aarch64-linux-android < %s -o - | FileCheck %s


define void @foo() nounwind uwtable safestack {
entry:
; CHECK: %[[TP:.*]] = call ptr @llvm.thread.pointer.p0()
; CHECK: %[[SPA0:.*]] = getelementptr i8, ptr %[[TP]], i32 72
; CHECK: %[[USP:.*]] = load ptr, ptr %[[SPA0]]
; CHECK: %[[USST:.*]] = getelementptr i8, ptr %[[USP]], i32 -16
; CHECK: store ptr %[[USST]], ptr %[[SPA0]]

  %a = alloca i8, align 8
  call void @Capture(ptr %a)

; CHECK: store ptr %[[USP]], ptr %[[SPA0]]
  ret void
}

declare void @Capture(ptr)
