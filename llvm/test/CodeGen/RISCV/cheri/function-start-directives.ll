;; Check that the directives at the start of the function are emitted in a sensible order
; RUN: %riscv64_cheri_purecap_llc -relocation-model=pic < %s | FileCheck %s
; RUN: %riscv64_cheri_purecap_llc -relocation-model=static < %s | FileCheck %s

target triple = "riscv64-unknown-freebsd13"

define dso_local noundef signext i32 @_Z4testv() local_unnamed_addr addrspace(200) uwtable personality ptr addrspace(200) @__gxx_personality_v0 {
entry:
  %call = invoke noundef signext i32 @_Z3foov(ptr addrspace(200) @_Z4testv)
          to label %return unwind label %lpad

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr addrspace(200), i32 }
          catch ptr addrspace(200) null
  %1 = extractvalue { ptr addrspace(200), i32 } %0, 0
  %2 = tail call ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200) %1) #2
  tail call void @__cxa_end_catch()
  br label %return

return:                                           ; preds = %entry, %lpad
  %retval.0 = phi i32 [ 3, %lpad ], [ %call, %entry ]
  ret i32 %retval.0
}

declare dso_local noundef signext i32 @_Z3foov(ptr addrspace(200)) local_unnamed_addr addrspace(200)

declare dso_local i32 @__gxx_personality_v0(...) addrspace(200)

declare dso_local ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200)) local_unnamed_addr addrspace(200)

declare dso_local void @__cxa_end_catch() local_unnamed_addr addrspace(200)

; CHECK:  .text
; CHECK-NEXT:  .attribute 4, 16
; CHECK-NEXT:  .attribute 5, "rv64i2p0_xcheri0p0"
; CHECK-NEXT:  .file "<stdin>"
; CHECK-NEXT:  .globl _Z4testv                        # -- Begin function _Z4testv
; CHECK-NEXT:  .p2align 2
; CHECK-NEXT:  .type _Z4testv,@function
; CHECK-NEXT: _Z4testv:                               # @_Z4testv
; CHECK-NEXT:   .L_Z4testv$local:
; CHECK-NEXT:   .type .L_Z4testv$local,@function
; CHECK-NEXT: .Lfunc_begin0:
; CHECK-NEXT:  .cfi_startproc
; CHECK-NEXT:  .cfi_personality 155, DW.ref.__gxx_personality_v0
; CHECK-NEXT:  .cfi_lsda 27, .Lexception0
; CHECK-NEXT: # %bb.0:                                # %entry

; CHECK: .Lfunc_end0:
; CHECK-NEXT: .size _Z4testv, .Lfunc_end0-_Z4testv
; CHECK-NEXT: .size .L_Z4testv$local, .Lfunc_end0-_Z4testv
; CHECK-NEXT: .cfi_endproc

; CHECK: .section .gcc_except_table
; CHECK: .word 12 # (landing pad is a capability)
; CHECK-NEXT: .chericap .L_Z4testv$local + .Ltmp2-.Lfunc_begin0 # jumps to .Ltmp2
