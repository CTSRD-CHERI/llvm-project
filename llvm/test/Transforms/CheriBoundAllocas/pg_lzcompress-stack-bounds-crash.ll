; RUN: %cheri_purecap_opt -opaque-pointers=0 -cheri-bound-allocas %s -o - -S -debug-only="cheri-bound-allocas" 2>&1 | %cheri_FileCheck %s -check-prefix DBG
; RUN: %riscv64_cheri_purecap_opt -opaque-pointers=0 -cheri-bound-allocas %s -o - -S -debug-only="cheri-bound-allocas" 2>&1 | %cheri_FileCheck %s -check-prefix DBG
; REQUIRES: asserts
; This crash was found compiling postgres (due to a missing depth limitation in CheriPurecapABI.cpp)

target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
target triple = "cheri-unknown-freebsd12-purecap"

@a = common local_unnamed_addr addrspace(200) global i32 0, align 4

; Function Attrs: noreturn nounwind
define signext i32 @b() local_unnamed_addr addrspace(200) #0 {
entry:
  %d = alloca i8, align 1, addrspace(200)
  %call = tail call signext i32 bitcast (i32 (...) addrspace(200)* @c to i32 () addrspace(200)*)() #3
  call void @llvm.lifetime.start.p200i8(i64 1, i8 addrspace(200)* nonnull %d)
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
  %ctrl.0 = phi i8 [ 0, %entry ], [ %ctrl.1, %if.end ]
  %cmp = icmp eq i8 %ctrl.0, 0
  %0 = load i32, i32 addrspace(200)* @a, align 4
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %while.cond
  %conv2 = trunc i32 %0 to i8
  store i8 %conv2, i8 addrspace(200)* %ctrlp.0, align 1
  br label %if.end

if.end:                                           ; preds = %while.cond, %if.then
  %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
  %ctrl.1 = phi i8 [ 1, %if.then ], [ %ctrl.0, %while.cond ]
  %or = or i32 %0, 1
  store i32 %or, i32 addrspace(200)* @a, align 4
  br label %while.cond
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p200i8(i64, i8 addrspace(200)* nocapture) addrspace(200) #1

declare signext i32 @c(...) local_unnamed_addr addrspace(200) #2

; TODO: can we avoid bounds for mutually recursing phis? Probably not worth the effort
; DBG-LABEL: Checking function b
; DBG-NEXT: cheri-bound-allocas: -Checking if phi needs stack bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:  -Checking if phi needs stack bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:   -Checking if phi needs stack bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:    -Checking if phi needs stack bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:     -Checking if phi needs stack bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:      -Checking if phi needs stack bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:       -Checking if phi needs stack bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:        -Checking if phi needs stack bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:         -Checking if phi needs stack bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:          -Checking if phi needs stack bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:           -reached max depth, assuming bounds needed.
; DBG-NEXT: cheri-bound-allocas:          -Adding stack bounds since phi user needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:         -Adding stack bounds since phi user needs bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:        -Adding stack bounds since phi user needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:       -Adding stack bounds since phi user needs bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:      -Adding stack bounds since phi user needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:     -Adding stack bounds since phi user needs bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:    -Adding stack bounds since phi user needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas:   -Adding stack bounds since phi user needs bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas:  -Adding stack bounds since phi user needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas: -Adding stack bounds since phi user needs bounds:   %ctrlp.1 = phi i8 addrspace(200)* [ bitcast (i32 () addrspace(200)* @b to i8 addrspace(200)*), %if.then ], [ %ctrlp.0, %while.cond ]
; DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %ctrlp.0 = phi i8 addrspace(200)* [ %d, %entry ], [ %ctrlp.1, %if.end ]
; DBG-NEXT: cheri-bound-allocas: -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.start.p200i8(i64 1, i8 addrspace(200)* nonnull %d)
; DBG-NEXT: cheri-bound-allocas: b: 1 of 2 users need bounds for   %d = alloca i8, align 1, addrspace(200)
; DBG-NEXT: b: setting bounds on stack alloca to 1  %d = alloca i8, align 1, addrspace(200)
