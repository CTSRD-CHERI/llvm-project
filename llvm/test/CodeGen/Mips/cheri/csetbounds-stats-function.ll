; RUN: rm -f %t.csv
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -O0 -o - \
; RUN:    -collect-csetbounds-output=%t.csv -collect-csetbounds-stats=csv \
; RUN:    -filetype=obj | llvm-objdump -t - | FileCheck -check-prefix SYMTAB %s
; RUN: FileCheck %s -input-file=%t.csv -check-prefix CSV

define internal i32 @maybe_inline(i32 %arg) unnamed_addr {
  entry:
    %result = add i32 %arg, 5
    ret i32 %result

}

define i32 @test_func() {
entry:
  %call = call i32 @external_fn()
  %call2 = call i32 @maybe_inline(i32 4)
  %result = add i32 %call, %call2
  ret i32 %result
}

declare i32 @external_fn()

; SYMTAB-LABEL: SYMBOL TABLE:
; SYMTAB-NEXT: 0000000000000000 l    df *ABS*		 00000000 csetbounds-stats-function.ll
; SYMTAB-NEXT: 0000000000000000 l     F .text		 00000010 maybe_inline
; SYMTAB-NEXT: 0000000000000000 l    d  .text		 00000000 .text
; SYMTAB-NEXT: 0000000000000000         *UND*		 00000000 external_fn
; SYMTAB-NEXT: 0000000000000010 g     F .text		 00000060 test_func
; SYMTAB-EMPTY:

; CSV: alignment_bits,size,kind,source_loc,compiler_pass,details
; CSV-NEXT: 2,16,c,"UNKNOWN","ELF symbol table","Function maybe_inline"
; CSV-NEXT: 2,96,c,"UNKNOWN","ELF symbol table","Function test_func"
; CSV-EMPTY:
