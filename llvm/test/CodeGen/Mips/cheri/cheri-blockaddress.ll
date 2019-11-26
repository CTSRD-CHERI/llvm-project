; RUN: %cheri_purecap_llc %s -o - | %cheri_FileCheck %s --check-prefix ASM
; RUN: %cheri_purecap_llc -filetype=obj %s -o - | llvm-readobj -r - | %cheri_FileCheck %s --check-prefix OBJ
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

; ASM-LABEL: .ent addrof_label_in_local
; ASM:     	cgetpcc	$c1
; ASM-NEXT:	lui	$1, %pcrel_hi(.Ltmp1+4)
; ASM-NEXT:	daddiu	$1, $1, %pcrel_lo(.Ltmp1+8)
; ASM-NEXT:	cincoffset	$c1, $c1, $1
; ASM:      .Ltmp1:
; ASM-NEXT: .LBB1_1:
; ASM: .end	addrof_label_in_local

; ASM-LABEL: addrof_label_in_static.b:
; ASM-NEXT: 	.chericap	addrof_label_in_static + .Ltmp0-addrof_label_in_static
; ASM-NEXT: 	.size	addrof_label_in_static.b, [[#CAP_SIZE]]

; The .o file should contain a relocation against the function with a constant addend (0x1c)
; OBJ:       Section (8) .rela.data {
; OBJ-NEXT:    0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE addrof_label_in_static 0x1C
; OBJ-NEXT:  }
