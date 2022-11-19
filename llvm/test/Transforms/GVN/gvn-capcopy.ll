; RUN: opt -gvn -S -o - %s | FileCheck %s

target datalayout = "e-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"

; CHECK-LABEL: DoCapabilityThing
; CHECK-NOT: undef
define void @DoCapabilityThing(i8 addrspace(200)* %cap)  {
entry:
  %cap.addr = alloca i8 addrspace(200)*, align 16
  store i8 addrspace(200)* %cap, i8 addrspace(200)** %cap.addr, align 16
  %cap.addr.0..sroa_cast = bitcast i8 addrspace(200)** %cap.addr to i64*
  %cap.addr.0.chunks.sroa.0.0.copyload = load i64, i64* %cap.addr.0..sroa_cast, align 16
  %cap.addr.8..sroa_raw_cast = bitcast i8 addrspace(200)** %cap.addr to i8*
  %cap.addr.8..sroa_raw_idx = getelementptr inbounds i8, i8* %cap.addr.8..sroa_raw_cast, i64 8
  %cap.addr.8..sroa_cast = bitcast i8* %cap.addr.8..sroa_raw_idx to i64*
  %cap.addr.8.chunks.sroa.2.0.copyload = load i64, i64* %cap.addr.8..sroa_cast
  call void @DoThing(i64 %cap.addr.0.chunks.sroa.0.0.copyload, i64 %cap.addr.8.chunks.sroa.2.0.copyload)
  ret void
}

declare void @DoThing(i64, i64)
