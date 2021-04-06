; RUN: llc < %s -mtriple=x86_64-apple-darwin | FileCheck --check-prefix=MACHO %s
; RUN: llc < %s -mtriple=x86_64-pc-linux | FileCheck --check-prefix=ELF %s
; RUN: llc < %s -mtriple=x86_64-pc-linux

;MACHO: .globl _offsetSym0
;MACHO-NOT: .alt_entry
;MACHO: .set _offsetSym0, _s
;MACHO: .globl _offsetSym1
;MACHO: .alt_entry _offsetSym1
;MACHO: .set _offsetSym1, _s+8

;ELF: .globl offsetSym0
;ELF-NOT: .alt_entry
;ELF: .set offsetSym0, s
;ELF-NEXT: .size offsetSym0, 12
;ELF: .globl offsetSym1
;ELF-NOT: .alt_entry
;ELF: .set offsetSym1, s+8
;ELF-NEXT: .size offsetSym1, 4

; Since @private_array has private linkage, AsmPrinter can set bounds to the underlying type.
; ELF-LABEL: .globl  private_array_plus_zero
; ELF-NEXT: .set  private_array_plus_zero, .Lprivate_array
; ELF-NEXT: .size  private_array_plus_zero, 1
; ELF-LABEL: .globl  private_array_plus_one
; ELF-NEXT: .set  private_array_plus_one, .Lprivate_array+1
; ELF-NEXT: .size  private_array_plus_one, 1

; ELF-LABEL: .globl  global_array_plus_zero
; ELF-NEXT: .set  global_array_plus_zero, global_array
; ELF-NEXT: .size  global_array_plus_zero, 16
; ELF-LABEL: .globl  global_array_plus_one
; ELF-NEXT: .set  global_array_plus_one, global_array+1
; ELF-NEXT: .size  global_array_plus_one, 15

%struct.S1 = type { i32, i32, i32 }

@s = global %struct.S1 { i32 31, i32 32, i32 33 }, align 4
@offsetSym0 = alias i32, i32* getelementptr inbounds (%struct.S1, %struct.S1* @s, i64 0, i32 0)
@offsetSym1 = alias i32, i32* getelementptr inbounds (%struct.S1, %struct.S1* @s, i64 0, i32 2)
@private_array = private global [16 x i8] zeroinitializer, align 4
@private_array_plus_zero = alias i8, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @private_array, i32 0, i32 0)
@private_array_plus_one = alias i8, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @private_array, i32 0, i32 1)
@global_array = global [16 x i8] zeroinitializer, align 4
@global_array_plus_zero = alias i8, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @global_array, i32 0, i32 0)
@global_array_plus_one = alias i8, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @global_array, i32 0, i32 1)
