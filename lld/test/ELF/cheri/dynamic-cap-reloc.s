# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o
# NOT: llvm-readobj -r %t.o
# RUN: ld.lld -preemptible-caprelocs=elf -shared %t.o -o %t-new.so -verbose -verbose-cap-relocs --relative-cap-relocs
# RUN: ld.lld -preemptible-caprelocs=elf -shared %t.o -o %t-old.so -verbose -verbose-cap-relocs --no-relative-cap-relocs
# RUN: llvm-readobj --dynamic --dyn-relocations --dyn-symbols --cap-relocs %t-new.so | FileCheck %s --check-prefixes DYN-RELOCS,DYN-RELOCS-NEW
# RUN: llvm-readobj --dynamic --dyn-relocations --dyn-symbols --cap-relocs %t-old.so | FileCheck %s --check-prefixes DYN-RELOCS,DYN-RELOCS-OLD
# Should have the same in-memory representation (just without the ELF relocations)
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs %t-new.so | FileCheck %s --check-prefixes DUMP,DUMP-NEW
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs %t-old.so | FileCheck %s --check-prefixes DUMP,DUMP-OLD

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
# DYN-RELOCS-OLD: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL{{$}}
# DYN-RELOCS-NEW: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL RELATIVE_CAPRELOCS {{$}}
# DYN-RELOCS-NEW: 0x000000007000C000 MIPS_CHERI___CAPRELOCS 0x3E0
# DYN-RELOCS-OLD: 0x000000007000C000 MIPS_CHERI___CAPRELOCS 0x20508
# DYN-RELOCS: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ 0x50

# DYN-RELOCS-LABEL:  Dynamic Relocations {
# the first four are for the __cap_relocs section (legacy mode)
# DYN-RELOCS-OLD-NEXT:  0x20508 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE -{{$}}
# DYN-RELOCS-OLD-NEXT:  0x20510 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE -{{$}}
# DYN-RELOCS-OLD-NEXT:  0x20530 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE -{{$}}
# DYN-RELOCS-OLD-NEXT:  0x20538 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE -{{$}}
# DYN-RELOCS-NEW-NEXT:  0x20490 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-OLD-NEXT:  0x30560 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEW-NEXT:  0x204D0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-OLD-NEXT:  0x305A0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# DYN-RELOCS-NEXT:   }

# DYN-RELOCS-LABEL: CHERI __cap_relocs [
# DYN-RELOCS-NEW-NEXT:   0x0204b0 (local_ref) Base: 0x10480 (bar+0) Length: 8 Perms: Function
# DYN-RELOCS-NEW-NEXT:   0x0204f0 (local_ref_with_addend) Base: 0x10480 (bar+4) Length: 8 Perms: Function
# DYN-RELOCS-OLD-NEXT:   0x030580 (local_ref) Base: 0x10500 (bar+0) Length: 8 Perms: Function
# DYN-RELOCS-OLD-NEXT:   0x0305c0 (local_ref_with_addend) Base: 0x10500 (bar+4) Length: 8 Perms: Function
# DYN-RELOCS-NEXT:  ]


# DUMP-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NEW-NEXT: 0x00000000000204b0	Base: bar (0x0000000000010480)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-NEW-NEXT: 0x00000000000204f0	Base: bar (0x0000000000010480)	Offset: 0x0000000000000004	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-OLD-NEXT: 0x0000000000030580	Base: bar (0x0000000000010500)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-OLD-NEXT: 0x00000000000305c0	Base: bar (0x0000000000010500)	Offset: 0x0000000000000004	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
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
