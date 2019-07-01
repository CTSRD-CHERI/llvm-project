; RUN: %cheri_llc %s -o - -relocation-model=static | %cheri_FileCheck %s
; RUN: %cheri_llc %s -o - -relocation-model=pic | %cheri_FileCheck %s
; RUN: %cheri_llc %s -o - -relocation-model=pic -legacy-cheri-cap-relocs | %cheri_FileCheck %s -check-prefix LEGACY
; RUN: %cheri_llc %s -o - -filetype=obj | llvm-readobj -r | %cheri_FileCheck -check-prefix=RELOCS %s
; ModuleID = 'global_init.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@a = common addrspace(200) global [5 x i32] zeroinitializer, align 4
@b = addrspace(200) global [3 x i32 addrspace(200)*] [
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 8) to i32 addrspace(200)*),
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 4) to i32 addrspace(200)*),
    i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)
  ], align 32
@c = addrspace(200) constant [3 x i32 addrspace(200)*] [
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 16) to i32 addrspace(200)*),
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 12) to i32 addrspace(200)*),
    i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)
  ], align 32

; CHECK:      .data
; CHECK-NEXT: .globl  b
; CHECK-NEXT: .p2align        {{5|4}}
; CHECK-NEXT: b:
; CHECK-NEXT: .chericap       a+8
; CHECK-NEXT: .chericap       a+4
; CHECK-NEXT: .chericap       a
; CHECK-NEXT: .size   b, {{96|48}}

; CHECK:      .section	.data.rel.ro,"aw",@progbits
; CHECK-NEXT: .globl  c
; CHECK-NEXT: .p2align        {{5|4}}
; CHECK-NEXT: c:
; CHECK-NEXT: .chericap       a+16
; CHECK-NEXT: .chericap       a+12
; CHECK-NEXT: .chericap       a
; CHECK-NEXT: .size   c, {{96|48}}

; RELOCS:       Section (5) .rela.data {
; RELOCS-NEXT:    0x0         R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0x8
; RELOCS-NEXT:    0x{{10|20}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0x4
; RELOCS-NEXT:    0x{{20|40}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0x0
; RELOCS-NEXT:  }
; RELOCS-NEXT:  Section (7) .rela.data.rel.ro {
; RELOCS-NEXT:    0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0x10
; RELOCS-NEXT:    0x{{10|20}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0xC
; RELOCS-NEXT:    0x{{20|40}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE a 0x0
; RELOCS-NEXT:  }



; LEGACY:      .data
; LEGACY-NEXT: .globl  b
; LEGACY-NEXT: .p2align        {{5|4}}
; LEGACY-NEXT: b:
; LEGACY-NEXT: .Ltmp0:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .Ltmp1:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .Ltmp2:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .size   b, {{96|48}}

; LEGACY:      .section	.data.rel.ro,"aw",@progbits
; LEGACY-NEXT: .globl  c
; LEGACY-NEXT: .p2align        {{5|4}}
; LEGACY-NEXT: c:
; LEGACY-NEXT: .Ltmp3:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .Ltmp4:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .Ltmp5:
; LEGACY-NEXT: .space [[#CAP_SIZE]]
; LEGACY-NEXT: .size   c, {{96|48}}

; LEGACY:	.section	__cap_relocs,"a",@progbits
; LEGACY:	.8byte	.Ltmp0
; LEGACY:	.8byte	a
; LEGACY:	.8byte	8
; LEGACY:	.space	16
; LEGACY:	.8byte	.Ltmp1
; LEGACY:	.8byte	a
; LEGACY:	.8byte	4
; LEGACY:	.space	16
; LEGACY:	.8byte	.Ltmp2
; LEGACY:	.8byte	a
; LEGACY:	.8byte	0
; LEGACY:	.space	16
; LEGACY:	.8byte	.Ltmp3
; LEGACY:	.8byte	a
; LEGACY:	.8byte	16
; LEGACY:	.space	16
; LEGACY:	.8byte	.Ltmp4
; LEGACY:	.8byte	a
; LEGACY:	.8byte	12
; LEGACY:	.space	16
; LEGACY:	.8byte	.Ltmp5
; LEGACY:	.8byte	a
; LEGACY:	.8byte	0
; LEGACY:	.space	16
