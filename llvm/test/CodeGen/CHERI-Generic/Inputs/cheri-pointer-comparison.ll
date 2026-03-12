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

define i32 @eq(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp eq ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ne(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ne ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ugt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ugt ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @uge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp uge ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ult(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ult ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ule(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ule ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sgt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sgt ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sge ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @slt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp slt ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sle(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sle ptr addrspace(200) %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @eq_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp eq ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ne_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ne ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ugt_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ugt ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @uge_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp uge ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ult_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ult ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ule_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ule ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sgt_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sgt ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sge_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sge ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @slt_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp slt ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sle_same(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sle ptr addrspace(200) %a, %a
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @eq_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp eq ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ne_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ne ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ugt_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ugt ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @uge_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp uge ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ult_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ult ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @ule_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp ule ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sgt_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sgt ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sge_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sge ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @slt_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp slt ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @sle_null(ptr addrspace(200) %a) nounwind {
  %cmp = icmp sle ptr addrspace(200) %a, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define ptr addrspace(200) @select_eq(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp eq ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ne(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ne ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ugt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ugt ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_uge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp uge ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ult(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ult ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ule(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ule ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sgt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sgt ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sge ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_slt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp slt ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sle(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sle ptr addrspace(200) %a, %b
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_eq_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp eq ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ne_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ne ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ugt_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ugt ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_uge_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp uge ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ult_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ult ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_ule_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp ule ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sgt_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sgt ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sge_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sge ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_slt_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp slt ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

define ptr addrspace(200) @select_sle_null(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
  %cmp = icmp sle ptr addrspace(200) %a, null
  %cond = select i1 %cmp, ptr addrspace(200) %a, ptr addrspace(200) %b
  ret ptr addrspace(200) %cond
}

declare i32 @func1() nounwind
declare i32 @func2() nounwind

define i32 @branch_eq(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp eq ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ne(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp ne ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ugt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp ugt ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_uge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp uge ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ult(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp ult ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ule(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp ule ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sgt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp sgt ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sge(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp sge ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_slt(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp slt ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sle(ptr addrspace(200) %a, ptr addrspace(200) %b) nounwind {
entry:
  %cmp = icmp sle ptr addrspace(200) %a, %b
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_eq_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp eq ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ne_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp ne ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ugt_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp ugt ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_uge_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp uge ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ult_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp ult ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_ule_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp ule ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sgt_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp sgt ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sge_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp sge ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_slt_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp slt ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}

define i32 @branch_sle_null(ptr addrspace(200) %a) nounwind {
entry:
  %cmp = icmp sle ptr addrspace(200) %a, null
  br i1 %cmp, label %if.then, label %if.end
if.then:
  %retval1 = tail call i32 @func1()
  ret i32 %retval1
if.end:
  %retval2 = tail call i32 @func2()
  ret i32 %retval2
}
