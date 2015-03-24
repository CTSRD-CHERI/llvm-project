; RUN: opt < %s -instcombine -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%myStruct = type { float, [3 x float], [4 x float], i32 }

; make sure that we are not crashing when creating an illegal type
define void @func(%myStruct addrspace(1)* nocapture %p) nounwind {
ST:
  %A = getelementptr inbounds %myStruct addrspace(1)* %p, i64 0
  %B = addrspacecast %myStruct addrspace(1)* %A to %myStruct*
  %C = getelementptr inbounds %myStruct* %B, i32 0, i32 1
  %D = getelementptr inbounds [3 x float]* %C, i32 0, i32 2
  %E = load float* %D, align 4
  %F = fsub float %E, undef
  ret void
}

@array = internal addrspace(3) global [256 x float] zeroinitializer, align 4
@scalar = internal addrspace(3) global float 0.000000e+00, align 4

; Make sure that GEPs are not reordered to before addrspacecasts.
define void @keep_necessary_addrspacecast(i64 %i, float** %out0, float** %out1) {
entry:
; CHECK-LABEL: @keep_necessary_addrspacecast
  %0 = getelementptr [256 x float]* addrspacecast ([256 x float] addrspace(3)* @array to [256 x float]*), i64 0, i64 %i
; CHECK: getelementptr [256 x float]* addrspacecast ([256 x float] addrspace(3)* @array to [256 x float]*), i64 0, i64 %i
  %1 = getelementptr [0 x float]* addrspacecast (float addrspace(3)* @scalar to [0 x float]*), i64 0, i64 %i
; CHECK: getelementptr [0 x float]* addrspacecast ([0 x float] addrspace(3)* bitcast (float addrspace(3)* @scalar to [0 x float] addrspace(3)*) to [0 x float]*), i64 0, i64 %i
  store float* %0, float** %out0, align 4
  store float* %1, float** %out1, align 4
  ret void
}

