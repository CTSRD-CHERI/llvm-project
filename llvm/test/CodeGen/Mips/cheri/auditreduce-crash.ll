; RUN: %cheri_llc -target-abi purecap -o /dev/null %s
; RUN: %cheri_llc -target-abi n64 -o /dev/null %s
; XFAIL: *
; REQUIRES: asserts
; ModuleID = 'auditreduce-crash.creduce.ll-reduced-simplified.bc'
source_filename = "auditreduce-crash.creduce.ll-output-4da6427.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.au_mask = type {}
%struct.tokenstr = type { i8, i8*, i64, %union.anon }
%union.anon = type { %struct.au_execarg_t }
%struct.au_execarg_t = type { i32, [128 x i8 addrspace(200)*] }

@maskp = external addrspace(200) global %struct.au_mask

define i32 @select_hdr32(%struct.tokenstr addrspace(200)* byval, i32 addrspace(200)* %optchkd) {
entry:
  %1 = getelementptr %struct.tokenstr, %struct.tokenstr addrspace(200)* %0, i64 0, i32 0
  %tok.sroa.1.0..sroa_idx = getelementptr i8, i8 addrspace(200)* %1
  %tok.sroa.1.0..sroa_cast = bitcast i8 addrspace(200)* %tok.sroa.1.0..sroa_idx to i16 addrspace(200)*
  %tok.sroa.1.0.copyload = load i16, i16 addrspace(200)* %tok.sroa.1.0..sroa_cast
  %call26 = call i32 @au_preselect(i16 %tok.sroa.1.0.copyload, %struct.au_mask addrspace(200)* @maskp, i32 3, i32 0)
  ret i32 0
}

define i32 @foo(i512* byval %x, %struct.tokenstr addrspace(200)* byval, i32 addrspace(200)* %optchkd) {
entry:
  %1 = getelementptr %struct.tokenstr, %struct.tokenstr addrspace(200)* %0, i64 0, i32 0
  %tok.sroa.1.0..sroa_idx = getelementptr i8, i8 addrspace(200)* %1
  %tok.sroa.1.0..sroa_cast = bitcast i8 addrspace(200)* %tok.sroa.1.0..sroa_idx to i16 addrspace(200)*
  %tok.sroa.1.0.copyload = load i16, i16 addrspace(200)* %tok.sroa.1.0..sroa_cast
  %call26 = call i32 @au_preselect(i16 %tok.sroa.1.0.copyload, %struct.au_mask addrspace(200)* @maskp, i32 3, i32 0)
  ret i32 0
}

declare i32 @au_preselect(i16, %struct.au_mask addrspace(200)*, i32, i32)

attributes #0 = { align=4 }
