# This error was found while debugging libcheri sandboxes: the compiler calls cheri_invoke,
# but in libcheri_invoke_cabi, only libcheri_ivoke had the .ent directive and cheri_invoke
# was therefore not marked as a data symbol. Add a check that LLD warns when using a %capcall20()
# relocation against a data symbol.
# For local symbols this will result in a __cap_relocs entry being added that is missing the
# function flag and therefore the capability in the captable will be non-executable.

# CheriBSD crt1.c will now trap during crt_init_globals() if this happens instead
# of creating a writable capability for the text segment.


# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -defsym=BUILD_LIBCHERI=1 -o %t-libcheri.o
# RUN: llvm-readobj -r %t.o | FileCheck %s -check-prefix RELOCS
# RELOCS-LABEL: Relocations [
# RELOCS-NEXT:   Section (3) .rela.text {
# RELOCS-NEXT:     0x0 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x4 R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x8 R_MIPS_CHERI_CAPCALL_HI16/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0xC R_MIPS_CHERI_CAPCALL_LO16/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x10 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x14 R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x18 R_MIPS_CHERI_CAPCALL_HI16/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x1C R_MIPS_CHERI_CAPCALL_LO16/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x20 R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x24 R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x28 R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x2C R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE libcheri_invoke 0x0
# RELOCS-NEXT:     0x30 R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x34 R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x38 R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x3C R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE cheri_invoke 0x0
# RELOCS-NEXT:     0x40 R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE weak_sym 0x0
# RELOCS-NEXT:   }

# Check that we get a warning both when linking static and dynamic:
# Static:
# RUN: ld.lld -o %t.exe %t.o %t-libcheri.o 2>&1 | FileCheck %s -check-prefixes CHECK,STATIC -implicit-check-not=warning:
# in a separate DSO:
# RUN: ld.lld -shared -o %t-libcheri.so %t-libcheri.o
# RUN: ld.lld -o %t2.exe %t-libcheri.so %t.o  2>&1 | FileCheck %s -check-prefixes CHECK,DYNAMIC -implicit-check-not=warning:

.ifdef BUILD_LIBCHERI
	.text
	.option pic0
	.global libcheri_invoke
	.global cheri_invoke
	.ent	libcheri_invoke
libcheri_invoke:
cheri_invoke:

	nop
	nop
	nop

	.end	libcheri_invoke
	# Also add size information for cheri_invoke to avoid linker warnings
.L.cheri_invoke.end:
	.size cheri_invoke, .L.cheri_invoke.end - cheri_invoke
.else

.global cheri_invoke
.global libcheri_invoke
.global __start
.global weak_sym
.weak weak_sym
.ent __start
.set noat
__start:
	# this one is fine (function symbol)
	clcbi $c1, %capcall20(libcheri_invoke)($cgp)
	clc $c1, $zero, %capcall(libcheri_invoke)($cgp)
	lui $at, %capcall_hi(libcheri_invoke)
	daddiu $at, $at, %capcall_lo(libcheri_invoke)

	# This should give an error
	clcbi $c1, %capcall20(cheri_invoke)($cgp)
# CHECK: ld.lld: warning: call relocation aginst non-function symbol {{(shared )?}}<unknown kind> cheri_invoke
# DYNAMIC-NEXT: >>> defined in {{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.so
# STATIC-NEXT: >>> defined in ({{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.o:(cheri_invoke))
# CHECK-NEXT: >>> referenced by function __start
# CHECK-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp.o:(__start))
# CHECK-EMPTY:
	clc $c1, $zero, %capcall(cheri_invoke)($cgp)
# CHECK-NEXT: ld.lld: warning: call relocation aginst non-function symbol {{(shared )?}}<unknown kind> cheri_invoke
# DYNAMIC-NEXT: >>> defined in {{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.so
# STATIC-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.o:(cheri_invoke))
# CHECK-NEXT: >>> referenced by function __start
# CHECK-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp.o:(__start))
# CHECK-EMPTY:
	lui $at, %capcall_hi(cheri_invoke)
# CHECK-NEXT: ld.lld: warning: call relocation aginst non-function symbol {{(shared )?}}<unknown kind> cheri_invoke
# DYNAMIC-NEXT: >>> defined in {{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.so
# STATIC-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.o:(cheri_invoke))
# CHECK-NEXT: >>> referenced by function __start
# CHECK-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp.o:(__start))
# CHECK-EMPTY:
	daddiu $at, $at, %capcall_lo(cheri_invoke)
# CHECK-NEXT: ld.lld: warning: call relocation aginst non-function symbol {{(shared )?}}<unknown kind> cheri_invoke
# DYNAMIC-NEXT: >>> defined in {{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.so
# STATIC-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp-libcheri.o:(cheri_invoke))
# CHECK-NEXT: >>> referenced by function __start
# CHECK-NEXT: >>> defined in  ({{.+}}capcall-reloc-against-data-symbol.s.tmp.o:(__start))
# CHECK-EMPTY:

	# Can't diagnose it for %captab:
	# this one is fine (function symbol)
	clcbi $c1, %captab20(libcheri_invoke)($cgp)
	clc $c1, $zero, %captab(libcheri_invoke)($cgp)
	lui $at, %captab_hi(libcheri_invoke)
	daddiu $at, $at, %captab_lo(libcheri_invoke)

	# This should give an error
	clcbi $c1, %captab20(cheri_invoke)($cgp)
	clc $c1, $zero, %captab(cheri_invoke)($cgp)
	lui $at, %captab_hi(cheri_invoke)
	daddiu $at, $at, %captab_lo(cheri_invoke)

	# No warning against a weak symbol:
	clc $c1, $zero, %capcall(weak_sym)($cgp)

.end __start
.endif
