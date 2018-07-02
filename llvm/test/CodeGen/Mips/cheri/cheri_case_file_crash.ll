; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o -
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=legacy -ignore-program-as-for-functions %s -o -
; ModuleID = '<stdin>'
source_filename = "bugpoint-output-4bb8cd9.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

declare i32 @__gxx_personality_v0(...)

declare void @b()

define void @c() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  invoke void @d()
          to label %1 unwind label %4

; <label>:1:                                      ; preds = %0
  invoke void @e()
          to label %2 unwind label %4

; <label>:2:                                      ; preds = %1
  invoke void @f()
          to label %3 unwind label %4

; <label>:3:                                      ; preds = %2
  invoke void @g()
          to label %6 unwind label %14

; <label>:4:                                      ; preds = %2, %1, %0
  %5 = landingpad { i8 addrspace(200)*, i32 }
          cleanup
  resume { i8 addrspace(200)*, i32 } undef

; <label>:6:                                      ; preds = %3
  invoke void @g()
          to label %7 unwind label %14

; <label>:7:                                      ; preds = %6
  invoke void @g()
          to label %8 unwind label %14

; <label>:8:                                      ; preds = %7
  invoke void @g()
          to label %9 unwind label %14

; <label>:9:                                      ; preds = %8
  invoke void @g()
          to label %10 unwind label %14

; <label>:10:                                     ; preds = %9
  invoke void @g()
          to label %11 unwind label %14

; <label>:11:                                     ; preds = %10
  invoke void @g()
          to label %12 unwind label %14

; <label>:12:                                     ; preds = %11
  invoke void @h()
          to label %13 unwind label %14

; <label>:13:                                     ; preds = %12
  invoke void (i8 addrspace(200)*, i32, ...) @i(i8 addrspace(200)* undef, i32 0)
          to label %19 unwind label %14

; <label>:14:                                     ; preds = %13, %12, %11, %10, %9, %8, %7, %6, %3
  %15 = landingpad { i8 addrspace(200)*, i32 }
          cleanup
  resume { i8 addrspace(200)*, i32 } undef

; <label>:16:                                     ; preds = %19
  %17 = landingpad { i8 addrspace(200)*, i32 }
          cleanup
  br label %18

; <label>:18:                                     ; preds = %18, %16
  br label %18

; <label>:19:                                     ; preds = %19, %13
  invoke void @b()
          to label %19 unwind label %16
}

declare void @i(i8 addrspace(200)*, i32, ...)

declare void @h()

declare void @d()

declare void @e()

declare void @f()

declare void @g()
