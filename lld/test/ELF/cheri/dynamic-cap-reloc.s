# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o
# NOT: llvm-readobj -r %t.o
# RUN: ld.lld -shared %t.o -o %t-new.so -verbose -verbose-cap-relocs
# RUN: llvm-readobj --dynamic --dyn-relocations --dyn-symbols --cap-relocs %t-new.so | FileCheck %s --check-prefix DYN-RELOCS
# Should have the same in-memory representation (just without the ELF relocations)
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs %t-new.so | FileCheck %s --check-prefix DUMP

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

preemptible_ref_with_addend:
.chericap foo + 2 # preemptible
.fill 16, 1, 0x13

local_ref_with_addend:
.chericap bar +4 # not preemptible
.fill 16, 1, 0x14


# We should have one R_MIPS_CHERI_CAPABILITY relocation and one entry in __cap_relocs:

# DYN-RELOCS-LABEL: DynamicSection [
# DYN-RELOCS: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL RELATIVE_CAPRELOCS {{$}}
# DYN-RELOCS: 0x000000007000C000 MIPS_CHERI___CAPRELOCS 0x3E0
# DYN-RELOCS: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ 0x50

# DYN-RELOCS-LABEL:  Dynamic Relocations {
# the first two are for the __cap_relocs section (legacy mode)
# DYN-RELOCS-NEXT:      0x{{20490|30520}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEXT:      0x{{204D0|30560}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEXT:   }

# DYN-RELOCS-LABEL: CHERI Capability Relocations [
# DYN-RELOCS-NEXT:    Section ({{.+}}) __cap_relocs {
# DYN-RELOCS-NEXT:      0x204B0 FUNC - 0x10480 [0x10480-0x10488]
# DYN-RELOCS-NEXT:      0x204F0 FUNC - 0x10484 [0x10480-0x10488]
# DYN-RELOCS-NEXT:    }
# DYN-RELOCS-NEXT:  ]


# DUMP-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NEXT:  OFFSET           TYPE    VALUE
# DUMP-NEXT:  00000000000204b0 FUNC    0000000000010480 [0000000000010480-0000000000010488]
# DUMP-NEXT:  00000000000204f0 FUNC    0000000000010484 [0000000000010480-0000000000010488]
# DUMP-EMPTY

# DUMP-LABEL: Contents of section .data:
# DUMP-NEXT:  00000000 00000000 cacacaca cacacaca
#             ^---- uninitialized preemptible_ref
# DUMP-NEXT:  11111111 11111111 11111111 11111111
# DUMP-NEXT:  cacacaca cacacaca cacacaca cacacaca
#             ^---- No addend for local_ref since we are using __cap_relocs
# DUMP-NEXT:  13131313 13131313 13131313 13131313
# DUMP-NEXT:  00000000 00000002 cacacaca cacacaca
#             ^---- addend 0x2 for preemptible_ref_with_addend
# DUMP-NEXT:  13131313 13131313 13131313 13131313
# DUMP-NEXT:  cacacaca cacacaca cacacaca cacacaca
#             ^---- No addend for local_ref_with_addend since we are using __cap_relocs
# DUMP-NEXT:  14141414 14141414 14141414 14141414
