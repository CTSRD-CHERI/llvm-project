; Disable the delay slot filler to avoid testing the reordering
; RUNs: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -O2 -o - -disable-mips-delay-filler
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -O2 -o - -disable-mips-delay-filler | %cheri_FileCheck %s -check-prefixes PLT,CHECK
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -O2 -o - -disable-mips-delay-filler | %cheri_FileCheck %s -check-prefixes PCREL,CHECK


; Check that $cgp is restored prior to calling other functions in the same TU
; after an external call (since that clobbers $cgp)

declare i32 @external_func() addrspace(200)
declare i32 @external_func2() addrspace(200)
declare dso_local i32 @external_local_func() addrspace(200)
@fn_ptr = internal unnamed_addr addrspace(200) global i32 () addrspace(200)* @external_func, align 32
@global_int = internal addrspace(200) global i64 4, align 4


define internal i64 @local_func_no_calls_but_uses_globals(i64 %arg) addrspace(200) nounwind noinline {
; $cgp stays the same -> no need to change
; CHECK-LABEL: local_func_no_calls_but_uses_globals:
; CHECK:       # %bb.0: # %entry
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c1, %captab20(global_int)($c{{1|26}})
; CHECK-NEXT:    cld	$1, $zero, 0($c1)
; CHECK-NEXT:    daddu $2, $4, $1
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %global_val = load i64, i64 addrspace(200)* @global_int
  %result = add i64 %arg, %global_val
  ret i64 %result
}

define internal i32 @local_func() addrspace(200) nounwind noinline {
  ; $cgp stays the same -> no need to change
  ; CHECK-LABEL: local_func:
  ; CHECK:       # %bb.0: # %entry
  ; CHECK-NEXT:    addiu $2, $zero, 1
  ; CHECK-NEXT:    cjr $c17
  ; CHECK-NEXT:    nop
entry:
  ret i32 1
}

define internal i32 @local_func_calls_external() addrspace(200) nounwind noinline {
; $cgp is portentially changed by the call to external_func -> save and restore
; CHECK-LABEL: local_func_calls_external:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; Make space for saving $cgp:
; PLT-NEXT:      csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; Save $cgp before first (potential) external call
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func)($c{{1|18}})
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; Restore $cgp after (potential) external call
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; PLT-NEXT:      clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %call = call i32 @external_func()
  ret i32 %call
}

define internal i32 @call_only_local() addrspace(200) nounwind noinline {
; $cgp stays the same -> no need to change (even though we call a local function
; that calls an external function)
; CHECK-LABEL: call_only_local:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; No need to save $cgp before call since it is local
; CHECK-NEXT:    clcbi $c12, %capcall20(local_func_calls_external)($c{{1|26}})
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; No need to restore $cgp after local call
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %result = call i32 @local_func_calls_external()
  ret i32 %result
}

; Need to restore $cgp each time we call an external function (since it might
; actually end up being in the same DSO and not have a trampoline that installs
; the correct $cgp
define i32 @call_two_external_funcs() addrspace(200) nounwind {
; CHECK-LABEL: call_two_external_funcs:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csd $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; Save $cgp before first (potential) external call
; $cgp only needs to be restored when not using the pc-relative ABI
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    move $16, $2
; Restore $cgp after (potential) external call
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func2)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    addu $2, $16, $2
; Restore $cgp after (potential) external call
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cld $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %call = call i32 @external_func()
  %call2 = call i32 @external_func2()
  %result = add i32 %call, %call2
  ret i32 %result
}

define i32 @call_global_then_local() addrspace(200) nounwind {
; CHECK-LABEL: call_global_then_local:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csd $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; Save $cgp before first (potential) external call
; $cgp only needs to be restored when not using the pc-relative ABI
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    move $16, $2
; $cgp only needs to be restored when not using the pc-relative ABI
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clcbi $c12, %capcall20(local_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; No need to restore $cgp after local call!
; CHECK-NEXT:    addu $16, $16, $2
; CHECK-NEXT:    clcbi $c12, %capcall20(external_local_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; No need to restore $cgp after dso_local call!
; CHECK-NEXT:    addu $2, $16, $2
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cld $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %call = call i32 @external_func()
  %call2 = call i32 @local_func()
  %result = add i32 %call, %call2
  ; DSO-local function should also not require saving $cgp
  %call3 = call i32 @external_local_func()
  %result2 = add i32 %result, %call3
  ret i32 %result2
}

define i32 @call_local_then_global() addrspace(200) nounwind {
; CHECK-LABEL: call_local_then_global:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csd $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(local_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; no need to restore $cgp after local call:
; CHECK-NEXT:    move $16, $2
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    addu $2, $16, $2
; Need to restore $cgp after first non-local call:
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cld $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %call = call i32 @local_func()
  %call2 = call i32 @external_func()
  %result = add i32 %call, %call2
  ret i32 %result
}

; No need to restore $cgp when calling a function pointer since that will always use a trampoline
define i32 @call_fn_ptr() addrspace(200) nounwind {
; CHECK-LABEL: call_fn_ptr:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csd $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; Save $cgp before calling fn pointer
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_func)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    clcbi $c1, %captab20(fn_ptr)($c18)
; CHECK-NEXT:    clc $c12, $zero, 0($c1)
; CHECK-NEXT:    move $16, $2
; Restore $cgp after calling external function (since it possibly clobbered $cgp)
; TODO: we could omit this since we know that fn_ptr will overwrite it anyway
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    addu $2, $16, $2
; Restore $cgp after calling fn pointer (since it definitely clobbered $cgp)
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cld $16, $zero, [[#STACKFRAME_SIZE - 8]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %call = call i32 @external_func()
  %fn = load i32 () addrspace(200)*, i32 () addrspace(200)* addrspace(200)* @fn_ptr, align 32
  %call2 = call i32 %fn()
  %result = add i32 %call, %call2
  ret i32 %result
}


; Some more test cases (not a call but a global variable, etc)
@global = local_unnamed_addr addrspace(200) global i8 123, align 8

declare void @external_call1() addrspace(200)
declare void @external_call2() addrspace(200)
declare i32 @external_i32() addrspace(200)

define i8 addrspace(200)* @access_global_after_external_call() addrspace(200) nounwind {
; CHECK-LABEL: access_global_after_external_call:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; We need to save $cgp before the call so it is moved to $c18 (callee-save)
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_call1)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    clcbi $c3, %captab20(global)($c18)
; restore $cgp after call to external function
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  call addrspace(200) void @external_call1()
  ret i8 addrspace(200)* @global
}

define void @call_two_functions() addrspace(200) nounwind {
; CHECK-LABEL: call_two_functions:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c18, $1
; save before calling external function
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    clcbi $c12, %capcall20(external_call1)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; restore after external 1
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clcbi $c12, %capcall20(external_call2)($c18)
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; restore after external 2
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  call addrspace(200) void @external_call1()
  call addrspace(200) void @external_call2()
  ret void
}

define i32 @not_needed_after_call(i32 %arg1, i32 %arg2) addrspace(200) nounwind {
; CHECK-LABEL: not_needed_after_call:
; CHECK:       # %bb.0: # %entry
; PCREL-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; PLT-NEXT:      cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csd $17, $zero, {{.+}}($c11) # 8-byte Folded Spill
; CHECK-NEXT:    csd $16, $zero, {{.+}}($c11) # 8-byte Folded Spill
; PLT-NEXT:      csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; Save $cgp for after external call:
; PLT-NEXT:      cmove $c18, $c26
; CHECK-NEXT:    sll $16, $5, 0
; CHECK-NEXT:    sll $17, $4, 0
; CHECK-NEXT:    clcbi $c12, %capcall20(external_call1)($c{{1|18}})
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    addu $2, $17, $16
; restore after potential external call
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; PLT-NEXT:      clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cld $16, $zero, {{.+}}($c11) # 8-byte Folded Reload
; CHECK-NEXT:    cld $17, $zero, {{.+}}($c11) # 8-byte Folded Reload
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  call addrspace(200) void @external_call1()
  %ret = add i32 %arg1, %arg2
  ret i32 %ret
}

define void @tailcall_external(i32 %arg1, i32 %arg2) addrspace(200) nounwind {
; CHECK-LABEL: tailcall_external:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; PLT-NEXT:      csc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; Save $cgp before external call
; PLT-NEXT:      cmove $c18, $c26
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c12, %capcall20(external_call1)($c{{1|18}})
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; restore $cgp after external call
; PLT-NEXT:      cmove $c26, $c18
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; PLT-NEXT:      clc $c18, $zero, [[#CAP_SIZE]]($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  tail call addrspace(200) void @external_call1()
  ret void
}


; TODO: why can't this be optimized to a jump?
define internal i32 @tailcall_local(i32 %arg1, i32 %arg2) addrspace(200) nounwind {
; CHECK-LABEL: tailcall_local:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -[[#STACKFRAME_SIZE:]]
; CHECK-NEXT:    csc $c17, $zero, 0($c11)
; no need to save/restore $cgp
; PCREL-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PCREL-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PCREL-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c12, %capcall20(local_func)($c{{1|26}})
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    nop
; CHECK-NEXT:    clc $c17, $zero, 0($c11)
; CHECK-NEXT:    cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
entry:
  %ret = tail call addrspace(200) i32 @local_func()
  ret i32 %ret
}



