# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# NOT: llvm-readobj -r %t.o
# RUN: ld.lld -preemptible-caprelocs=elf -shared %t.o -o %t.so -verbose -verbose-cap-relocs
# RUN: llvm-readobj -dyn-relocations -dyn-symbols %t.so | FileCheck %s -check-prefix DYN-RELOCS
# RUN: llvm-objdump -C -s -t --section=.data --section=__cap_relocs %t.so | FileCheck %s -check-prefix DUMP

.text
.global foo
.ent foo
foo:
  nop
.end foo

.hidden bar
.p2align 6
.ent bar
bar:
  nop
  nop
.end bar

.data
preemptible_ref:
.chericap foo # preemptible

.fill 16, 1, 0x11

local_ref:
.chericap bar # not preemptible
.fill 16, 1, 0x13

.ifdef NOTYET
preemptible_ref_with_addend:
.chericap foo + 2 # preemptible
.fill 16, 1, 0x13

local_ref_with_addend:
.chericap bar + 2 # not preemptible
.fill 16, 1, 0x14
.endif


# We should have one R_MIPS_CHERI_CAPABILITY relocation and one entry in __cap_relocs:

# DYN-RELOCS-LABEL:  Dynamic Relocations {
# the first two are for the __cap_relocs section
# DYN-RELOCS-NEXT:    0x30000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-NEXT:    0x30008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-NEXT:    0x20000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# DYN-RELOCS-NEXT:  }

# DUMP-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NEXT: 0x0000000000020020	Base: bar (0x0000000000010040)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)

# DUMP-LABEL: Contents of section .data:
# DUMP-NEXT:  20000 cacacaca cacacaca cacacaca cacacaca
#                       ^---- uninitialized preemptible_ref
# DUMP-NEXT:  20010 11111111 11111111 11111111 11111111
# DUMP-NEXT:  20020 cacacaca cacacaca cacacaca cacacaca
#                       ^---- uninitialized local_ref
# DUMP-NEXT:  20030 13131313 13131313 13131313 13131313
