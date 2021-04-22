; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP
; Check that selects and branches using capability compares use unsigned comparisons.
; NGINX has a loop with (void*)-1 as a sentinel value which was never entered due to this bug.
; Original issue: https://github.com/CTSRD-CHERI/llvm/issues/199
; Fixed upstream in https://reviews.llvm.org/D70917 (be15dfa88fb1ed94d12f374797f98ede6808f809)
;
; Original source code showing this surprising behaviour (for CHERI-MIPS):
; int
; main(void)
; {
;         void *a, *b;
;
;         a = (void *)0x12033091e;
;         b = (void *)0xffffffffffffffff;
;
;         if (a < b) {
;                 printf("ok\n");
;                 return (0);
;         }
;
;         printf("surprising result\n");
;         return (1);
; }
;
; Morello had a similar code generation issue for selects, where a less than
; generated a csel instruction using a singed predicate instead of the unsigned one:
; void *select_lt(void *p1, void *p2) {
;   return p1 < p2 ? p1 : p2;
; }
; See https://git.morello-project.org/morello/llvm-project/-/issues/22


define i32 @lt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @le(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @gt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i8 addrspace(200)* @select_lt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_le(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_gt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

declare i32 @func1() nounwind noreturn
declare i32 @func2() nounwind noreturn

define i32 @branch_lt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind noreturn {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  unreachable
if.end:
  %retval2 = tail call i32 @func2()
  unreachable
}

define i32 @branch_le(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind noreturn {
entry:
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  unreachable
if.end:
  %retval2 = tail call i32 @func2()
  unreachable
}

define i32 @branch_gt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind noreturn {
entry:
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  unreachable
if.end:
  %retval2 = tail call i32 @func2()
  unreachable
}

define i32 @branch_ge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind noreturn {
entry:
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  unreachable
if.end:
  %retval2 = tail call i32 @func2()
  unreachable
}
