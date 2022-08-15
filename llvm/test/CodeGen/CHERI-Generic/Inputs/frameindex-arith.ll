; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s

; Check that we can fold the GEP (PTRADD) into the FrameIndex calculation
; rather than emitting two instructions.

; Contains an explicit @llvm.cheri.cap.bounds.set so CheriBoundAllocas sees the
; use as safe and doesn't interfere by inserting bounds on the FrameIndex
; before the GEP/PTRADD.
define void @foo() nounwind {
  %x = alloca [2 x i8], align 1, addrspace(200)
  %x_plus_1 = getelementptr inbounds [2 x i8], [2 x i8] addrspace(200)* %x, iCAPRANGE 0, iCAPRANGE 1
  %p = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.iCAPRANGE(i8 addrspace(200)* %x_plus_1, iCAPRANGE 0)
  call void @bar(i8 addrspace(200)* %p)
  ret void
}

declare void @bar(i8 addrspace(200)*)

declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
