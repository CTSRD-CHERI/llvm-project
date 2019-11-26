# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o 2>&1 | FileCheck %s "--implicit-check-not=warning:" --check-prefix WARNING
# RUN: llvm-readobj -r %t.o | FileCheck %s
## Check that we can create R_MIPS_CHERI_CAPABILITY relocations against symbols with not yet known sizes
## See https://github.com/CTSRD-CHERI/llvm-project/issues/353
	.type	lmp_head,@object
	.data
	.globl	lmp_head
lmp_head:
	.chericap	lmp_head-2
	.size	lmp_head, 16

	.globl  external_symbol
	.globl  lmp_head_extern
lmp_head_extern:
	.chericap	external_symbol+1
	.size	lmp_head_extern, 16

.Llmp_head_local:
	.chericap	.Llmp_head_local+1
	.size	.Llmp_head_local, 16

.hidden .Llmp_head_local_unsized
.Llmp_head_local_unsized:
	.chericap	.Llmp_head_local_unsized+1
# WARNING: [[@LINE-1]]:12: warning: creating a R_MIPS_CHERI_CAPABILITY relocation against an unsized defined symbol: .Llmp_head_local_unsized.

.section .otherdata, "aw", %progbits
.Lreloc_text_local:
.chericap .Lfoo_start + 4
# WARNING: [[@LINE-1]]:11: warning: creating a R_MIPS_CHERI_CAPABILITY relocation against an unsized defined symbol: .Lfoo_start.
.Lreloc_text_global:
.chericap foo + 4

.text
.global foo
.ent foo
foo:
  nop
.Lfoo_start:
  nop
.end foo

# CHECK-LABEL: Relocations [
# CHECK-NEXT:   Section (4) .rela.data {
# CHECK-NEXT:     0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE lmp_head 0xFFFFFFFFFFFFFFFE
# CHECK-NEXT:     0x10 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE external_symbol 0x1
# CHECK-NEXT:     0x20 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .Llmp_head_local 0x1
# CHECK-NEXT:     0x30 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .Llmp_head_local_unsized 0x1
# CHECK-NEXT:   }
# CHECK-NEXT:   Section (6) .rela.otherdata {
# CHECK-NEXT:     0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .Lfoo_start 0x4
# CHECK-NEXT:     0x10 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x4
# CHECK-NEXT:   }
