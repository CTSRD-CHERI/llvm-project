; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: %riscv64_cheri_purecap_llc -verify-machineinstrs -o - %s | FileCheck %s
; Regression test for a crash found while compiling functions with frames greater than 2048 bytes
; after the April 2021 upstream LLVM merge (caused by a mismerge of 02ffbac844e01df2c95dfcb3117213211fe2226d).
; Assertion failed: (RealDef != nullptr && "Must have at least 1 Def"), function scavengeVReg

%struct.large = type { [2014 x i8] }
%struct.too.large = type { [2018 x i8] }

declare i32 @use_i128(i128 addrspace(200)*)
declare i32 @use_large(%struct.large addrspace(200)*)
declare i32 @use_too_large(%struct.too.large addrspace(200)*)

define void @just_below_threshold() local_unnamed_addr addrspace(200) nounwind {
; CHECK-LABEL: just_below_threshold:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset csp, csp, -2032
; CHECK-NEXT:    sc cra, 2016(csp) # 16-byte Folded Spill
; CHECK-NEXT:    cincoffset csp, csp, -32
; CHECK-NEXT:    cincoffset ca0, csp, 2032
; CHECK-NEXT:    csetbounds ca0, ca0, 16
; CHECK-NEXT:    ccall use_i128
; CHECK-NEXT:    cincoffset ca0, csp, 16
; CHECK-NEXT:    csetbounds ca0, ca0, 2014
; CHECK-NEXT:    ccall use_large
; CHECK-NEXT:    cincoffset csp, csp, 32
; CHECK-NEXT:    lc cra, 2016(csp) # 16-byte Folded Reload
; CHECK-NEXT:    cincoffset csp, csp, 2032
; CHECK-NEXT:    ret
entry:
  %obj1 = alloca i128, align 16, addrspace(200)
  %obj2 = alloca %struct.large, align 16, addrspace(200)
  call i32 @use_i128(i128 addrspace(200)* %obj1)
  call i32 @use_large(%struct.large addrspace(200)* %obj2)
  ret void
}

define void @just_above_threshold() local_unnamed_addr addrspace(200) nounwind {
; CHECK-LABEL: just_above_threshold:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset csp, csp, -2032
; CHECK-NEXT:    sc cra, 2016(csp) # 16-byte Folded Spill
; CHECK-NEXT:    cincoffset csp, csp, -48
; CHECK-NEXT:    lui a0, 1
; CHECK-NEXT:    cincoffset ca0, csp, a0
; CHECK-NEXT:    cincoffset ca0, ca0, -2048
; CHECK-NEXT:    csetbounds ca0, ca0, 16
; CHECK-NEXT:    ccall use_i128
; CHECK-NEXT:    cincoffset ca0, csp, 24
; CHECK-NEXT:    csetbounds ca0, ca0, 2018
; CHECK-NEXT:    ccall use_too_large
; CHECK-NEXT:    cincoffset csp, csp, 48
; CHECK-NEXT:    lc cra, 2016(csp) # 16-byte Folded Reload
; CHECK-NEXT:    cincoffset csp, csp, 2032
; CHECK-NEXT:    ret
entry:
  %obj1 = alloca i128, align 16, addrspace(200)
  %obj2 = alloca %struct.too.large, addrspace(200)
  call i32 @use_i128(i128 addrspace(200)* %obj1)
  call i32 @use_too_large(%struct.too.large addrspace(200)* %obj2)
  ret void
}
