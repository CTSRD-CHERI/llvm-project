; RUN: not --crash %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - 2>&1 | FileCheck %s
; CHECK: false &&  "This is wrong and needs a different fix!"

; RUNTODO: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - -filetype=obj | llvm-objdump -d -t -r -
; RUNTODO: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - -filetype=obj | llvm-readobj -r -
; RUNTODO: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - | FileCheck %s -check-prefixes ASM,ASM-NEW
; RUNTODO: %cheri_purecap_llc -cheri-cap-table-abi=legacy %s -o - | FileCheck %s -check-prefixes ASM,ASM-LEGACY
; See address-of-label-crash.c in clang/test/CodeGen/cheri for the C source code

; Function Attrs: noinline nounwind optnone
define i32 @addrof_label_in_static() addrspace(200) {
entry:
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* @addrof_label_in_static.b, align 16
  br label %indirectgoto

label1:                                           ; preds = %indirectgoto
  ret i32 2

indirectgoto:                                     ; preds = %entry
  %indirect.goto.dest = phi i8 addrspace(200)* [ %0, %entry ]
  indirectbr i8 addrspace(200)* %indirect.goto.dest, [label %label1]
}

@addrof_label_in_static.b = internal addrspace(200) global i8 addrspace(200)* blockaddress(@addrof_label_in_static, %label1), align 16

; Create a global containing the address of the label:
; ASM-LABEL: .ent addrof_label_in_static
; ASM-NEW:    clcbi	$c1, %captab20(addrof_label_in_static.b)($c26)
; ASM-LEGACY: ld	$1, %got_page(addrof_label_in_static.b)($1)
; ASM-LEGACY-NEXT: daddiu	$1, $1, %got_ofst(addrof_label_in_static.b)
; ASM-LEGACY-NEXT: cfromddc	$c1, $1
; ASM-LEGACY-NEXT: csetbounds	$c1, $c1, {{16|32}}
; ASM-LEGACY-NEXT: clc	$c1, $zero, 0($c1)
; ASM:     .Ltmp0:
; ASM-NEXT: .LBB0_1:
; ASM: .end	addrof_label_in_static


; Function Attrs: noinline nounwind optnone
define i32 @addrof_label_in_local() addrspace(200) {
entry:
  %d = alloca i8 addrspace(200)*, align 16, addrspace(200)
  store i8 addrspace(200)* blockaddress(@addrof_label_in_local, %label2), i8 addrspace(200)* addrspace(200)* %d, align 16
  %0 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %d, align 16
  br label %indirectgoto

label2:                                           ; preds = %indirectgoto
  ret i32 2

indirectgoto:                                     ; preds = %entry
  %indirect.goto.dest = phi i8 addrspace(200)* [ %0, %entry ]
  indirectbr i8 addrspace(200)* %indirect.goto.dest, [label %label2]
}

; ASM-LABEL: .ent addrof_label_in_local
; ASM-NEW:    clcbi	$c1, %capcall20(.Ltmp1)($c26)
; ASM-LEGACY: ld	$1, %got_page(.Ltmp1)($1)
; ASM-LEGACY: daddiu	$1, $1, %got_ofst(.Ltmp1)
; ASM-LEGACY: cgetpccsetoffset $c2, $1
; ASM:      .Ltmp1:
; ASM-NEXT: .LBB1_1:
; ASM: .end	addrof_label_in_local

; ASM-LABEL: addrof_label_in_static.b:
; ASM-NEXT: .chericap	.Ltmp0
