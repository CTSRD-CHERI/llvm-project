; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/function-alias-size.ll
; RUN: llc -mtriple=riscv32 --relocation-model=pic -target-abi il32pc64f -mattr=+xcheri,+cap-mode,+f %s -o - < %s | FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=riscv32 --relocation-model=pic -target-abi il32pc64f -mattr=+xcheri,+cap-mode,+f %s -o - -filetype=obj < %s | llvm-objdump --syms -r - | FileCheck %s --check-prefix=OBJDUMP
; The MIPS backend asserts emitting a relocation against an unsized but defined
; function-type global, which was happening with destructor aliases:
; The _ZN*D1Ev destructor is emitted as an alias for the defined _ZN*D2Ev destructor,
; and did not have size information, which triggered the assertion after the April 2021 merge.
; Check that we emit size information for function aliases:

@a = constant i8 addrspace(200)* bitcast (void () addrspace(200)* @_ZN3fooD1Ev to i8 addrspace(200)*)
@_ZN3fooD1Ev = alias void (), void () addrspace(200)* @_ZN3fooD2Ev
define void @_ZN3fooD2Ev() addrspace(200) nounwind {
; ASM-LABEL: _ZN3fooD2Ev:
; ASM:       # %bb.0:
; ASM-NEXT:    ret
  ret void
}

@two_ints = private global {i32, i32} {i32 1, i32 2}
@elem0 = alias i32, getelementptr({i32, i32}, {i32, i32}*  @two_ints, i32 0, i32 0)
@elem1 = alias i32, getelementptr({i32, i32}, {i32, i32}*  @two_ints, i32 0, i32 1)

; UTC_ARGS: --disable
; ASM: .size _ZN3fooD2Ev, .Lfunc_end0-_ZN3fooD2Ev

; ASM-LABEL: .Ltwo_ints:
; ASM-NEXT: .{{4byte|word}} 1
; ASM-NEXT: .{{4byte|word}} 2
; ASM-NEXT: .size .Ltwo_ints, 8

; The function alias symbol should have the same size expression:
; ASM-LABEL: .globl _ZN3fooD1Ev
; ASM-NEXT: .type _ZN3fooD1Ev,@function
; ASM-NEXT: .set _ZN3fooD1Ev, _ZN3fooD2Ev
; ASM-NEXT: .size _ZN3fooD1Ev, .Lfunc_end0-_ZN3fooD2Ev

; But for the aliases using a GEP, we have to subtract the offset:
; ASM-LABEL: .globl elem0
; ASM-NEXT:  .set elem0, .Ltwo_ints
; ASM-NEXT:  .size elem0, 4
; ASM-LABEL: .globl elem1
; ASM-NEXT:  .set elem1, .Ltwo_ints+4
; ASM-NEXT:  .size elem1, 4

; Check that the ELF st_size value was set correctly:
; OBJDUMP-LABEL: SYMBOL TABLE:
; OBJDUMP-NEXT: {{0+}}0 l    df *ABS* {{0+}} function-alias-size.ll
; OBJDUMP-DAG: {{0+}}0 g     F .text [[SIZE:[0-9a-f]+]] _ZN3fooD2Ev
; OBJDUMP-DAG: {{0+}}0 g     O .data.rel.ro {{0+(10|8)}} a
; OBJDUMP-DAG: {{0+}}0 g     F .text [[SIZE]] _ZN3fooD1Ev
; elem1 should have a size of 4 and not 8:
; OBJDUMP-DAG: {{0+}}0 g     O .{{s?}}data {{0+}}4 elem0
; OBJDUMP-DAG: {{0+}}4 g     O .{{s?}}data {{0+}}4 elem1
