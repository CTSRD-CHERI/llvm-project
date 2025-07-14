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
# DYN-RELOCS: 0x000000007000C000 MIPS_CHERI___CAPRELOCS 0x418
# DYN-RELOCS: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ 0x50

# DYN-RELOCS-LABEL:  Dynamic Relocations {
# the first two are for the __cap_relocs section (legacy mode)
# DYN-RELOCS-NEXT:      0x{{204D0|30560}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEXT:      0x{{20510|305A0}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEXT:   }

# DYN-RELOCS-LABEL: CHERI __cap_relocs [
# DYN-RELOCS-NEXT:     0x0204f0 (local_ref) Base: 0x104c0 (bar) Length: 8 Perms: Function
# DYN-RELOCS-NEXT:     0x020530 (local_ref_with_addend) Base: 0x104c0 (bar+4) Length: 8 Perms: Function
# DYN-RELOCS-NEXT:  ]


# DUMP-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NEXT:  0x00000000000204f0	Base: bar (0x00000000000104c0)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-NEXT:  0x0000000000020530	Base: bar (0x00000000000104c0)	Offset: 0x0000000000000004	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
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
