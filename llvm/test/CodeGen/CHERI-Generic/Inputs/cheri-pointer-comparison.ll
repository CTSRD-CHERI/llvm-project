; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP
; This series of tests serves two purposes.
; The first purpose is to check that we generate efficient code for all
; capability comparisons, conditional branches and conditional selects.
; The second purpose is to check that we generate code that honours the
; signedness of the IR (which is always unsigned when emitting IR for C
; langage-level pointers, whereas __intcap uses the signedness of the type).
; NGINX has a loop with (void*)-1 as a sentinel value which was never entered
; due to this bug.
; Original issue: https://github.com/CTSRD-CHERI/llvm/issues/199
; Fixed upstream in https://reviews.llvm.org/D70917
; (be15dfa88fb1ed94d12f374797f98ede6808f809)
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
; generated a csel instruction using a singed predicate instead of the unsigned
; one:
; void *select_lt(void *p1, void *p2) {
;   return p1 < p2 ? p1 : p2;
; }
; See https://git.morello-project.org/morello/llvm-project/-/issues/22

define i32 @eq(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp eq i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ne(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ne i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ugt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @uge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ult(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ule(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sgt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sgt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sge i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @slt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp slt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sle(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sle i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @eq_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp eq i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ne_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp ne i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ugt_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp ugt i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @uge_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp uge i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ult_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp ult i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ule_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp ule i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sgt_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp sgt i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sge_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp sge i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @slt_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp slt i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sle_null(i8 addrspace(200)* %a) nounwind {
  %cmp = icmp sle i8 addrspace(200)* %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i8 addrspace(200)* @select_eq(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp eq i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ne(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ne i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ugt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_uge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ult(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ule(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sgt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sgt i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sge i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_slt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp slt i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sle(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sle i8 addrspace(200)* %a, %b
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_eq_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp eq i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ne_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ne i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ugt_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ugt i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_uge_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp uge i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ult_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ult i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_ule_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp ule i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sgt_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sgt i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sge_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sge i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_slt_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp slt i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

define i8 addrspace(200)* @select_sle_null(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
  %cmp = icmp sle i8 addrspace(200)* %a, null
  %cond = select i1 %cmp, i8 addrspace(200)* %a, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %cond
}

declare i32 @func1() nounwind
declare i32 @func2() nounwind

define i32 @branch_eq(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp eq i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ne(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ne i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ugt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_uge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ult(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ule(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sgt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp sgt i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sge(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp sge i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_slt(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp slt i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sle(i8 addrspace(200)* %a, i8 addrspace(200)* %b) nounwind {
entry:
  %cmp = icmp sle i8 addrspace(200)* %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_eq_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp eq i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ne_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp ne i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ugt_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp ugt i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_uge_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp uge i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ult_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp ult i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ule_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp ule i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sgt_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp sgt i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sge_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp sge i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_slt_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp slt i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sle_null(i8 addrspace(200)* %a) nounwind {
entry:
  %cmp = icmp sle i8 addrspace(200)* %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}
