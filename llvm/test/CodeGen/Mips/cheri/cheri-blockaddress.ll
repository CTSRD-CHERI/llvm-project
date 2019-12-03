; RUN: %cheri_purecap_llc %s -o - | %cheri_FileCheck %s --check-prefix ASM
; RUN: %cheri_purecap_llc -filetype=obj %s -o - | llvm-readobj -r - | %cheri_FileCheck %s --check-prefix OBJ
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - -filetype=obj | llvm-objdump -d -t -r - | %cheri_FileCheck %s -check-prefix DUMP
; See address-of-label-crash.c in clang/test/CodeGen/cheri for the C source code

; Function Attrs: noinline nounwind optnone
define i32 @addrof_label_in_static() addrspace(200) {
entry:
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* @addrof_label_in_static.b, align 32
  br label %indirectgoto

label1:                                           ; preds = %indirectgoto
  ret i32 2

indirectgoto:                                     ; preds = %entry
  %indirect.goto.dest = phi i8 addrspace(200)* [ %0, %entry ]
  indirectbr i8 addrspace(200)* %indirect.goto.dest, [label %label1]
}

@addrof_label_in_static.b = internal addrspace(200) global i8 addrspace(200)* blockaddress(@addrof_label_in_static, %label1), align 32

; Create a global containing the address of the label:
; ASM-LABEL: .ent addrof_label_in_static
; ASM:         clcbi	$c1, %captab20(addrof_label_in_static.b)($c1)
; ASM:     .Ltmp0:
; ASM-NEXT: .LBB0_1:
; ASM: .end	addrof_label_in_static


; Function Attrs: noinline nounwind optnone
define i32 @addrof_label_in_local() addrspace(200) {
entry:
  %d = alloca i8 addrspace(200)*, align 16, addrspace(200)
  store i8 addrspace(200)* blockaddress(@addrof_label_in_local, %label2), i8 addrspace(200)* addrspace(200)* %d, align 32
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %d, align 32
  br label %indirectgoto
label2:                                           ; preds = %indirectgoto
  ret i32 2
indirectgoto:                                     ; preds = %entry
  %indirect.goto.dest = phi i8 addrspace(200)* [ %0, %entry ]
  indirectbr i8 addrspace(200)* %indirect.goto.dest, [label %label2]
}

; ASM-LABEL: addrof_label_in_local:
; ASM:       # %bb.0: # %entry
; ASM-NEXT:    cincoffset $c11, $c11, -[[#CAP_SIZE]]
; ASM-NEXT:    .cfi_def_cfa_offset [[#CAP_SIZE]]
; Generate a pc-relative blockaddress:
; ASM-NEXT:    lui $1, %pcrel_hi(.Ltmp1-8)
; ASM-NEXT:    daddiu $1, $1, %pcrel_lo(.Ltmp1-4)
; ASM-NEXT:    cgetpccincoffset $c1, $1
; ASM-NEXT:    cjr $c1
; ASM-NEXT:    csc $c1, $zero, 0($c11)
; ASM-NEXT:  .Ltmp1: # Block address taken
; ASM-NEXT:  .LBB1_1: # %label2
; ASM-NEXT:    addiu $2, $zero, 2
; ASM-NEXT:    cjr $c17
; ASM-NEXT:    cincoffset $c11, $c11, [[#CAP_SIZE]]
; ASM: .end	addrof_label_in_local

; ASM-LABEL: addrof_label_in_static.b:
; ASM-NEXT: 	.chericap	addrof_label_in_static + .Ltmp0-addrof_label_in_static
; ASM-NEXT: 	.size	addrof_label_in_static.b, [[#CAP_SIZE]]

; The .o file should contain a relocation against the function with a constant addend (0x1c)
; OBJ:       Section (8) .rela.data {
; OBJ-NEXT:    0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE addrof_label_in_static 0x1C
; OBJ-NEXT:  }


; DUMP-LABEL: addrof_label_in_local:
; DUMP-NEXT:        4a 6b 5f {{f|e}}0 	cincoffset	$c11, $c11, -[[#CAP_SIZE]]
; DUMP-NEXT:        3c 01 00 00 	lui	$1, 0
; 3 instructions -> value should be 12:
; DUMP-NEXT:        64 21 00 0c 	daddiu	$1, $1, [[PCREL_LOWER:12]]
; DUMP-NEXT:        48 01 0c ff 	cgetpccincoffset $c1, $1
; DUMP-NEXT:        48 01 1f ff 	cjr	$c1
; DUMP-NEXT:        f8 2b 00 00 	csc	$c1, $zero, 0($c11)
; Check that the jump target address is correct
; DUMP-NEXT:        24 02 00 02 	addiu	$2, $zero, 2
; DUMP-NEXT:        48 11 1f ff 	cjr	$c17
; DUMP-NEXT:        4a 6b 58 {{1|2}}0 	cincoffset	$c11, $c11, [[#CAP_SIZE]]
