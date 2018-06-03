; REQUIRES: asserts
; RUN: %cheri_purecap_llc -o - -O2 -cheri-cap-table-abi=pcrel -debug-only=mips-lower %s 2>&1 | %cheri_FileCheck %s -check-prefixes CHECK,OPT
; RUN: %cheri_purecap_llc -o - -O1 -cheri-cap-table-abi=pcrel -debug-only=mips-lower %s 2>&1 | %cheri_FileCheck %s -check-prefixes CHECK,NOOPT
; RUN: %cheri_purecap_llc -o /dev/null -O2 -cheri-cap-table-abi=pcrel -debug-only=mips-lower %s 2>&1 | FileCheck %s -check-prefix DEBUG-OUTPUT-CHECK

@fn = common local_unnamed_addr addrspace(200) global void () addrspace(200)* null, align 32


declare i8 addrspace(200)* @variadic(i8 addrspace(200)* %first_arg, ...)
declare i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)
declare i8 addrspace(200)* @many_cap_args(i8 addrspace(200)* %arg1, i8 addrspace(200)* %arg2, i8 addrspace(200)* %arg3, i8 addrspace(200)* %arg4,
                                          i8 addrspace(200)* %arg5, i8 addrspace(200)* %arg6, i8 addrspace(200)* %arg7, i8 addrspace(200)* %arg8,
                                          i8 addrspace(200)* %arg9, i8 addrspace(200)* %arg10, i8 addrspace(200)* %arg11, i8 addrspace(200)* %arg12)


; Check that the debug output prints
; DEBUG-OUTPUT-CHECK:      Clearing $c13 in call_one_arg_from_many_arg(is varargs: 0) callee = {{t[0-9]+}}: i64 = GlobalAddress<i8 addrspace(200)* (i8 addrspace(200)*)* @one_arg> 0
; DEBUG-OUTPUT-CHECK-NEXT: Lowering return for function with $c13 live-in: call_one_arg_from_many_arg(is varargs: 0)
; DEBUG-OUTPUT-CHECK-NEXT: Clearing $c13 in call_one_arg_from_variadic_with_va_start(is varargs: 1) callee = {{t[0-9]+}}: i64 = GlobalAddress<i8 addrspace(200)* (i8 addrspace(200)*)* @one_arg> 0
; DEBUG-OUTPUT-CHECK-NEXT: Lowering return for function with $c13 live-in: call_one_arg_from_variadic_with_va_start(is varargs: 1)
; DEBUG-OUTPUT-CHECK-NEXT: Clearing $c13 in call_one_arg_from_variadic_without_va_start(is varargs: 1) callee = {{t[0-9]+}}: i64 = GlobalAddress<i8 addrspace(200)* (i8 addrspace(200)*)* @one_arg> 0
; DEBUG-OUTPUT-CHECK-NEXT: Lowering return for function with $c13 live-in: call_one_arg_from_variadic_without_va_start(is varargs: 1)
; DEBUG-OUTPUT-CHECK-NEXT: Clearing $c13 in call_variadic_no_onstack_from_varargs(is varargs: 1) callee = {{t[0-9]+}}: i64 = GlobalAddress<i8 addrspace(200)* (i8 addrspace(200)*, ...)* @variadic> 0
; DEBUG-OUTPUT-CHECK-NEXT: Lowering return for function with $c13 live-in: call_variadic_no_onstack_from_varargs(is varargs: 1)
; DEBUG-OUTPUT-CHECK-NEXT: Clearing $c13 in call_variadic_no_onstack_from_many_args(is varargs: 0) callee = {{t[0-9]+}}: i64 = GlobalAddress<i8 addrspace(200)* (i8 addrspace(200)*, ...)* @variadic> 0
; DEBUG-OUTPUT-CHECK-NEXT: Lowering return for function with $c13 live-in: call_variadic_no_onstack_from_many_args(is varargs: 0)

; CHECK-LABEL: .text
; CHECK-LABEL: .file

define void @call_variadic_one_onstack_long(i8 addrspace(200)* %in_arg1) {
; CHECK-LABEL: call_variadic_one_onstack_long:
; Verify that $c13 is used for one on-stack i64:
; CHECK:         csetbounds [[VARARG_TMP:\$c[0-9]+]], $c11, 8
; CHECK-NEXT:    ori $1, $zero, 65495
; CHECK:         candperm $c13, [[VARARG_TMP]], $1
; CHECK: .end call_variadic_one_onstack_long
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg, i64 42)
  ret void
}

define void @call_variadic_one_onstack_cap(i8 addrspace(200)* %in_arg1) {
; CHECK-LABEL: call_variadic_one_onstack_cap:
; Verify that $c13 is used for one on-stack cap:
; CHECK:         csetbounds [[VARARG_TMP:\$c[0-9]+]], $c11, [[$CAP_SIZE]]
; CHECK-NEXT:    ori $1, $zero, 65495
; CHECK-NEXT:    candperm $c13, [[VARARG_TMP]], $1
; CHECK: .end call_variadic_one_onstack_cap
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %second_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 87)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg, i8 addrspace(200)* %second_arg)
  ret void
}

; Function Attrs: nounwind
define void @call_variadic_no_onstack(i8 addrspace(200)* %in_arg1) {
; CHECK-LABEL: call_variadic_no_onstack:
; No need for a cgenull here since it is already null
; OPT-NOT:   $c13
; but at -01 we still clear just in case
; NOOPT:    cgetnull $c13
; CHECK:      .end call_variadic_no_onstack
entry:
  ; increment by a number the compiler will never emit for anything else (77) to be able to see which registers are used:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg)
  ret void
}

; This function should not set $c13 to zero since it does not have on-stack-arguments
define void @call_nonvariadic_one_arg(i8 addrspace(200)* %in_arg1) {
; CHECK-LABEL: call_nonvariadic_one_arg:
; $c13 should remain unchanged since we don't have on-stack args
; OPT-NOT:   $c13
; but at -01 we still clear just in case
; NOOPT:    cgetnull $c13
; CHECK: .end call_nonvariadic_one_arg
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)
  ret void
}

define void @call_one_arg_from_two_arg(i8 addrspace(200)* %in_arg1, i8 addrspace(200)* %in_arg2) {
; CHECK-LABEL: call_one_arg_from_two_arg:
; $c13 should remain unchanged since we don't have on-stack args
; OPT-NOT:   $c13
; but at -01 we still clear just in case
; NOOPT:    cgetnull $c13
; CHECK: .end call_one_arg_from_two_arg
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)
  ret void
}

define void @call_nonvariadic_many_args(i8 addrspace(200)* %in_arg1) {
; CHECK-LABEL: call_nonvariadic_many_args:
; Test that we are passing all the arguments after $c10 on the stack and not zeroing $c13
; CHECK:    cincoffset $c1, $c3, 17
; CHECK-NEXT:    cincoffset $c4, $c3, 27
; CHECK-NEXT:    cincoffset $c5, $c3, 37
; CHECK-NEXT:    cincoffset $c6, $c3, 47
; CHECK-NEXT:    cincoffset $c7, $c3, 57
; CHECK-NEXT:    cincoffset $c8, $c3, 67
; CHECK-NEXT:    cincoffset $c9, $c3, 77
; CHECK-NEXT:    cincoffset $c10, $c3, 87
; CHECK-NEXT:    cincoffset $c2, $c3, 97
; CHECK-NEXT:    cincoffset $c12, $c3, 107
; CHECK-NEXT:    cincoffset $c13, $c3, 117
; CHECK-NEXT:    cincoffset $c3, $c3, 127
; CHECK-NEXT:    csc $c2, $zero, 0($c11)
; CHECK-NEXT:    csc $c3, $zero, [[@EXPR $CAP_SIZE * 3]]($c11)
; CHECK-NEXT:    csc $c13, $zero, [[@EXPR $CAP_SIZE * 2]]($c11)
; CHECK-NEXT:    csc $c12, $zero, [[@EXPR $CAP_SIZE * 1]]($c11)
; CHECK-NEXT:    clcbi $c12, %capcall20(many_cap_args)($c26)
; CHECK-NEXT:    csetbounds $c2, $c11, [[@EXPR $CAP_SIZE * 4]]
; CHECK-NEXT:    ori $1, $zero, 65495
; CHECK-NEXT:    candperm $c13, $c2, $1
; CHECK-NEXT:    cjalr $c12, $c17
; CHECK-NEXT:    cmove $c3, $c1
; CHECK-NEXT:    clc $c17, $zero, {{.*}}($c11)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    cincoffset $c11, $c11, {{.*}}
; CHECK:         .end call_nonvariadic_many_args

entry:
  %arg1 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 17)
  %arg2 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 27)
  %arg3 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 37)
  %arg4 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 47)
  %arg5 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 57)
  %arg6 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 67)
  %arg7 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %arg8 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 87)
  %arg9 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 97)
  %arg10 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 107)
  %arg11 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 117)
  %arg12 = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 127)
  %0 = call i8 addrspace(200)* @many_cap_args(i8 addrspace(200)* %arg1, i8 addrspace(200)* %arg2, i8 addrspace(200)* %arg3, i8 addrspace(200)* %arg4,
                                              i8 addrspace(200)* %arg5, i8 addrspace(200)* %arg6, i8 addrspace(200)* %arg7, i8 addrspace(200)* %arg8,
                                              i8 addrspace(200)* %arg9, i8 addrspace(200)* %arg10, i8 addrspace(200)* %arg11, i8 addrspace(200)* %arg12)
  ret void
}

define void @call_one_arg_from_many_arg(i8 addrspace(200)* %in_arg1, i8 addrspace(200)* %arg2, i8 addrspace(200)* %arg3, i8 addrspace(200)* %arg4,
                                        i8 addrspace(200)* %arg5, i8 addrspace(200)* %arg6, i8 addrspace(200)* %arg7, i8 addrspace(200)* %arg8,
                                        i8 addrspace(200)* %arg9, i8 addrspace(200)* %arg10, i8 addrspace(200)* %arg11, i8 addrspace(200)* %arg12) {
; This cgetnull should remain here since we are calling a function without on-stack args from one with
; CHECK-LABEL: call_one_arg_from_many_arg:
; CHECK:       cincoffset	$c3, $c3, 77
; CHECK:            cjalr   $c12, $c17
; Clear $c13 in branch-delay slot
; CHECK-NEXT:       cgetnull $c13
; Also need to clear $c13 on return since this function has on-stack args
; TODO: we know $c13 is already null so this is not strictly required but probably not worth optimizing
; CHECK-NEXT:       cgetnull $c13
; CHECK:       .end	call_one_arg_from_many_arg
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)
  ret void
}

define void @call_one_arg_from_variadic_with_va_start(i8 addrspace(200)* %in_arg1, ...) {
; This cgenull should remain since we have a valid $c13 that should not leak to the other function
; CHECK-LABEL: call_one_arg_from_variadic_with_va_start:
; CHECK:       cincoffset	$c3, $c3, 77
; CHECK:            cjalr   $c12, $c17
; Clear $c13 in branch-delay slot
; CHECK-NEXT:       cgetnull $c13
; Also need to clear $c13 on return since this function has on-stack args
; TODO: we know $c13 is already null so this is not strictly required but probably not worth optimizing
; CHECK-NEXT:       cgetnull $c13
; CHECK:       .end	call_one_arg_from_variadic_with_va_start
entry:
  %ap = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %ap1 = bitcast i8 addrspace(200)* addrspace(200)* %ap to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 32, i8 addrspace(200)* %ap1) #1
  call void @llvm.va_start.p200i8(i8 addrspace(200)* %ap1)
  ; call void @llvm.va_copy.p200i8.p200i8(i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @va_cpy to i8 addrspace(200)*), i8 addrspace(200)* %0)

  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)

  call void @llvm.va_end.p200i8(i8 addrspace(200)* %ap1)
  call void @llvm.lifetime.end.p200i8(i64 32, i8 addrspace(200)* %ap1) #1
  ret void
}

define void @call_one_arg_from_variadic_without_va_start(i8 addrspace(200)* %in_arg1, ...) {
; We are calling a non-variadic function from a varargs one we should clear $c13:
; (This should be the case even if the function doesn't use va_start (i.e. doesn't mark $c13 as live-in)
; CHECK-LABEL: call_one_arg_from_variadic_without_va_start:
; CHECK:       cincoffset	$c3, $c3, 77
; CHECK:            cjalr   $c12, $c17
; Clear $c13 in branch-delay slot
; CHECK-NEXT:       cgetnull $c13
; Also need to clear $c13 on return since this function has on-stack args
; TODO: we know $c13 is already null so this is not strictly required but probably not worth optimizing
; CHECK-NEXT:       cgetnull $c13
; CHECK:       .end	call_one_arg_from_variadic_without_va_start
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %first_arg)
  ret void
}

; The following two should clear $c13

define void @call_variadic_no_onstack_from_varargs(i8 addrspace(200)* %in_arg1, ...) {
; Calling a varargs function without onstack args from a varargs function should clear $c13
; CHECK-LABEL: call_variadic_no_onstack_from_varargs:
; CHECK:       cincoffset	$c3, $c3, 77
; CHECK:            cjalr   $c12, $c17
; Clear $c13 in branch-delay slot
; CHECK-NEXT:       cgetnull $c13
; Also need to clear $c13 on return since this function has on-stack args
; TODO: we know $c13 is already null so this is not strictly required but probably not worth optimizing
; CHECK-NEXT:       cgetnull $c13
; CHECK:       .end	call_variadic_no_onstack_from_varargs
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg)
  ret void
}

define void @call_variadic_no_onstack_from_many_args(i8 addrspace(200)* %arg1, i8 addrspace(200)* %arg2, i8 addrspace(200)* %arg3, i8 addrspace(200)* %arg4,
                                                     i8 addrspace(200)* %arg5, i8 addrspace(200)* %arg6, i8 addrspace(200)* %arg7, i8 addrspace(200)* %arg8,
                                                     i8 addrspace(200)* %arg9, i8 addrspace(200)* %arg10, i8 addrspace(200)* %arg11, i8 addrspace(200)* %arg12) {
; Calling a varargs function without onstack args from a function with on-stack args should clear $c13
; CHECK-LABEL: call_variadic_no_onstack_from_many_args:
; CHECK:       cincoffset	$c3, $c3, 77
; CHECK:            cjalr   $c12, $c17
; Clear $c13 in branch-delay slot
; CHECK-NEXT:       cgetnull $c13
; Also need to clear $c13 on return since this function has on-stack args
; TODO: we know $c13 is already null so this is not strictly required but probably not worth optimizing
; CHECK-NEXT:       cgetnull $c13
; CHECK:       .end	call_variadic_no_onstack_from_many_args
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %arg1, i64 77)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg)
  ret void
}


define void @call_variadic_onstack_and_no_stack_fn_from_nostack_fn(i8 addrspace(200)* %in_arg1) {
; Calling a varargs function without onstack args from a varargs function should clear $c13
; CHECK-LABEL: call_variadic_onstack_and_no_stack_fn_from_nostack_fn:
; CHECK:       cincoffset	$c3, $c18, 77
; CHECK:       candperm        $c13, $c1, $1
; Since the variadic fn clears $c13 on return we should not have another cgetnull here!
; OPT-NOT:   $c13
; CHECK:       cincoffset      $c3, $c18, 97
; OPT-NOT:   $c13
; CHECK:       .end	call_variadic_onstack_and_no_stack_fn_from_nostack_fn
entry:
  %first_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 77)
  %second_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 87)
  %0 = call i8 addrspace(200)* (i8 addrspace(200)*, ...) @variadic(i8 addrspace(200)* %first_arg, i8 addrspace(200)* %second_arg)
  %third_arg = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in_arg1, i64 97)
  %1 = call i8 addrspace(200)* @one_arg(i8 addrspace(200)* %third_arg)
  ret void
}




declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)*, i64)

; Function Attrs: nounwind
declare void @llvm.lifetime.start.p200i8(i64, i8 addrspace(200)* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.va_start.p200i8(i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.va_copy.p200i8.p200i8(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.va_end.p200i8(i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end.p200i8(i64, i8 addrspace(200)* nocapture) #1
