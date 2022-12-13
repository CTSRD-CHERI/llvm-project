; RUN: not llvm-as %s -o /dev/null 2>&1 | FileCheck %s
; RUN: llvm-as %s -data-layout=P199 -o - | llvm-dis - -o - | FileCheck %s -check-prefix PROGAS199


; Check that variables in a nonzero program address space 199 can be used in a invoke instruction

define i8 @test_invoke(ptr %fnptr0, ptr addrspace(199) %fnptr199) personality ptr addrspace(199) @__gxx_personality_v0 {
  %explicit_as_0 = invoke addrspace(0) i8 %fnptr0(i32 0) to label %ok unwind label %lpad
  %explicit_as_42 = invoke addrspace(199) i8 %fnptr199(i32 0) to label %ok unwind label %lpad
  ; The following is only okay if the program address space is 199:
  %no_as = invoke i8 %fnptr199(i32 0) to label %ok unwind label %lpad
  ; CHECK: invoke-nonzero-program-addrspace.ll:[[@LINE-1]]:22: error: '%fnptr199' defined with type 'ptr addrspace(199)' but expected 'ptr'
ok:
  ret i8 0
lpad:
    %exn = landingpad {ptr, i32}
            cleanup
    unreachable
}

declare i32 @__gxx_personality_v0(...)


; PROGAS199:  target datalayout = "P199"
; PROGAS199:  define i8 @test_invoke(ptr %fnptr0, ptr addrspace(199) %fnptr199) addrspace(199) personality ptr addrspace(199) @__gxx_personality_v0 {
; PROGAS199:    %explicit_as_0 = invoke addrspace(0) i8 %fnptr0(i32 0)
; PROGAS199:    %explicit_as_42 = invoke addrspace(199) i8 %fnptr199(i32 0)
; PROGAS199:    %no_as = invoke addrspace(199) i8 %fnptr199(i32 0)
; PROGAS199:    ret i8 0
; PROGAS199:  }
