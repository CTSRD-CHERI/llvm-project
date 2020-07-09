; Run with delay slot filling disabled since this makes the test fragile by moving instructions after scheduling
; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=beri -mattr=+beri -O2 -disable-mips-delay-filler %s -o - | FileCheck %s -check-prefixes CHECK,BERI
; With -mcpu=mips4 we get lots of pipeline bubbles:
; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=mips4 -O2 -disable-mips-delay-filler %s -o - | FileCheck %s -check-prefixes CHECK,MIPS4
; Check that adding the -cheri flag enables BERI scheduling:
; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=mips4 -mattr=+cheri,+cheri128 -O2 -disable-mips-delay-filler %s -o - | FileCheck %s -check-prefixes CHECK,BERI

; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=mips4 -filetype=obj %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-MIPS4
; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=beri -filetype=obj %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-BERI
; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=mips4 -mattr=+cheri128 -filetype=obj %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-CHERI128
; FLAGS:                Flags [
; FLAGS-CHERI128-SAME:   (0x30C10005)
; FLAGS-BERI-SAME:       (0x30BE0005)
; FLAGS-MIPS4-SAME:      (0x30000005)
; FLAGS-NEXT:             EF_MIPS_ARCH_4 (0x30000000)
; FLAGS-NEXT:             EF_MIPS_CPIC (0x4)
; FLAGS-BERI-NEXT:        EF_MIPS_MACH_BERI (0xBE0000)
; FLAGS-CHERI128-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
; FLAGS-NEXT:             EF_MIPS_NOREORDER (0x1)

; FLAGS-NEXT:           ]

; Originally generated from:
; long test2(const void *a1, const void *a2) {
;   const long l1 = *(const long *)a1;
;   const long l2 = *(const long *)a2;
;   const long diff = (const char *)a1 - (const char *)a2;
;   return l1 - l2 + diff;
; }

; Check that there are two instructions between load and use (this does not happen with -mcpu=mips4)
; CHECK-LABEL: test2:
; BERI:      ld	$1, 0($4)
; BERI-NEXT: ld	$2, 0($5)
; BERI-NEXT: dsubu $3, $4, $5
; BERI-NEXT: dsubu $1, $1, $2
; BERI-NEXT: daddu $2, $1, $3


; With -mcpu=mips we use the load result immediately
; MIPS4:      dsubu $1, $4, $5
; MIPS4-NEXT: ld $2, 0($5)
; MIPS4-NEXT: ld $3, 0($4)
; MIPS4-NEXT: dsubu $2, $3, $2
; MIPS4-NEXT: daddu $2, $2, $1

define i64 @test2(i8* %a1, i8* %a2) nounwind {
entry:
  %sub.ptr.lhs.cast = ptrtoint i8* %a1 to i64
  %sub.ptr.rhs.cast = ptrtoint i8* %a2 to i64
  %0 = bitcast i8* %a1 to i64*
  %1 = load i64, i64* %0, align 8
  %2 = bitcast i8* %a2 to i64*
  %3 = load i64, i64* %2, align 8
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub = sub nsw i64 %1, %3
  %add = add nsw i64 %sub, %sub.ptr.sub
  ret i64 %add
}

; Originally generated from:
; long test3(const void *a1, const void *a2, long extra1, long extra2) {
;   const long l1 = *(const long *)a1;
;   const long l2 = *(const long *)a2;
;   const long extra = extra1 + extra2;
;   return l1 - l2 + extra;
; }

; CHECK-LABEL: test3:
; Check that there are two instructions between load of $1/$2 and the use:
; BERI:      ld	$1, 0($4)
; BERI-NEXT: ld	$2, 0($5)
; BERI-NEXT: daddu	$3, $6, $7
; BERI-NEXT: dsubu	$1, $1, $2
; BERI-NEXT: daddu	$2, $1, $3

; But -mcpu=mips4 just uses it immediately
; MIPS4:      daddu $1, $6, $7
; MIPS4-NEXT: ld $2, 0($5)
; MIPS4-NEXT: ld $3, 0($4)
; MIPS4-NEXT: dsubu $2, $3, $2
; MIPS4-NEXT: daddu $2, $2, $1
define i64 @test3(i8* %a1, i8* %a2, i64 signext %extra1, i64 signext %extra2) nounwind {
entry:
  %0 = bitcast i8* %a1 to i64*
  %1 = load i64, i64* %0, align 8
  %2 = bitcast i8* %a2 to i64*
  %3 = load i64, i64* %2, align 8
  %add = add nsw i64 %extra1, %extra2
  %sub = sub nsw i64 %1, %3
  %add1 = add nsw i64 %sub, %add
  ret i64 %add1
}
