; RUN: %cheri_opt -S -gvn %s | FileCheck %s
; ModuleID = 'union2.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define i32 @bar(i8 addrspace(200)* inreg %u.coerce) #0 {
entry:
  %u.sroa.0.sroa.0 = alloca i8 addrspace(200)*, align 32, addrspace(200)
  store i8 addrspace(200)* %u.coerce, i8 addrspace(200)* addrspace(200)* %u.sroa.0.sroa.0, align 32
  %u.sroa.0.sroa.0.0.x.sroa_cast2 = bitcast i8 addrspace(200)* addrspace(200)* %u.sroa.0.sroa.0 to i32 addrspace(200)*
  %u.sroa.0.sroa.0.0.u.sroa.0.sroa.0.0.u.sroa.0.0.u.sroa.0.0. = load i32, i32 addrspace(200)* %u.sroa.0.sroa.0.0.x.sroa_cast2, align 32
  ; Check that GVN doesn't attempt to turn a store of a capability followed by
  ; a load of an integer into something else (this behaviour broke unions)
  ; CHECK:   %call = tail call i32 @foo(i32 signext %u.sroa.0.sroa.0.0.u.sroa.0.sroa.0.0.u.sroa.0.0.u.sroa.0.0.)
  %call = tail call i32 @foo(i32 signext %u.sroa.0.sroa.0.0.u.sroa.0.sroa.0.0.u.sroa.0.0.u.sroa.0.0.) #3
  ret i32 %call
}

declare i32 @foo(i32 signext) #1

attributes #0 = { nounwind }
attributes #1 = { }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang f8b93d4881438c4368509c2a579f59e3c9d11c61) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm 99d44e3898b2909744731e598e56b8ee44891d15)"}
