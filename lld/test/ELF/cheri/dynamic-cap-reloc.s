# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o
# NOT: llvm-readobj -r %t.o
# RUN: ld.lld -preemptible-caprelocs=elf -shared %t.o -o %t-new.so -verbose -verbose-cap-relocs --relative-cap-relocs
# RUN: ld.lld -preemptible-caprelocs=elf -shared %t.o -o %t-old.so -verbose -verbose-cap-relocs --no-relative-cap-relocs
# RUN: llvm-readobj --dynamic -dyn-relocations -dyn-symbols --cap-relocs %t-new.so | FileCheck %s -check-prefixes DYN-RELOCS,DYN-RELOCS-NEW
# RUN: llvm-readobj --dynamic -dyn-relocations -dyn-symbols --cap-relocs %t-old.so | FileCheck %s -check-prefixes DYN-RELOCS,DYN-RELOCS-OLD
# Should have the same in-memory representation (just without the ELF relocations)
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs %t-old.so | FileCheck %s -check-prefix DUMP
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs %t-new.so | FileCheck %s -check-prefix DUMP

# Test that we can also emit R_CHERI_CAPABILITY relocations for local symbols (even though it is stupid)
# RUN: ld.lld -preemptible-caprelocs=elf -local-caprelocs=elf -shared %t.o -o %t.so -verbose -verbose-cap-relocs
# RUN: llvm-readobj -dyn-relocations -dyn-symbols %t.so | FileCheck %s -check-prefixes=R_CAPABILITY_FOR_ALL
# RUN: llvm-objdump --cap-relocs -s --section=.data --section=__cap_relocs -t %t.so | FileCheck %s -check-prefix DUMP-R_CAPABILITY_FOR_ALL

# RUN: ld.lld -pie -preemptible-caprelocs=elf -local-caprelocs=elf %t.o -o %t.exe
# RUN: llvm-readobj -dyn-relocations -dyn-symbols %t.exe | FileCheck %s -check-prefixes=R_CAPABILITY_FOR_ALL
# RUN: llvm-objdump --cap-relocs -s -t --section=.data --section=__cap_relocs %t.exe | FileCheck %s -check-prefix DUMP-R_CAPABILITY_FOR_ALL

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

# DYN-RELOCS-LABEL:  Dynamic Relocations {
# the first four are for the __cap_relocs section (legacy mode)
# DYN-RELOCS-OLD-NEXT:  0x20000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-OLD-NEXT:  0x20008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-OLD-NEXT:  0x20028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-OLD-NEXT:  0x20030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0
# DYN-RELOCS-NEXT:      0x30000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# DYN-RELOCS-NEXT:      0x30040 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# DYN-RELOCS-NEXT:   }

# DYN-RELOCS-LABEL: DynamicSection [
# DYN-RELOCS-OLD: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL{{$}}
# DYN-RELOCS-NEW: 0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL RELATIVE_CAPRELOCS {{$}}
# DYN-RELOCS: 0x000000007000C000 MIPS_CHERI___CAPRELOCS0x20000
# DYN-RELOCS: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ0x50

# DYN-RELOCS-LABEL: CHERI __cap_relocs [
# DYN-RELOCS-NEXT:   0x030020 (local_ref) Base: 0x10040 (bar+0) Length: 8 Perms: Function
# DYN-RELOCS-NEXT:   0x030060 (local_ref_with_addend) Base: 0x10040 (bar+4) Length: 8 Perms: Function
# DYN-RELOCS-NEXT:  ]


# DUMP-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NEXT: 0x0000000000030020	Base: bar (0x0000000000010040)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-NEXT: 0x0000000000030060	Base: bar (0x0000000000010040)	Offset: 0x0000000000000004	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
# DUMP-EMPTY

# DUMP-LABEL: Contents of section .data:
# DUMP-NEXT:  30000 00000000 00000000 cacacaca cacacaca
#                       ^---- uninitialized preemptible_ref
# DUMP-NEXT:  30010 11111111 11111111 11111111 11111111
# DUMP-NEXT:  30020 cacacaca cacacaca cacacaca cacacaca
#                        ^---- No addend for local_ref since we are using __cap_relocs
# DUMP-NEXT:  30030 13131313 13131313 13131313 13131313
# DUMP-NEXT:  30040 00000000 00000002 cacacaca cacacaca
#                        ^---- addend 0x2 for preemptible_ref_with_addend
# DUMP-NEXT:  30050 13131313 13131313 13131313 13131313
# DUMP-NEXT:  30060 cacacaca cacacaca cacacaca cacacaca
#                        ^---- No addend for local_ref_with_addend since we are using __cap_relocs
# DUMP-NEXT:  30070 14141414 14141414 14141414 14141414


# R_CAPABILITY_FOR_ALL:      Dynamic Relocations {
# R_CAPABILITY_FOR_ALL-NEXT:   0x20020 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE bar 0x0
# R_CAPABILITY_FOR_ALL-NEXT:   0x20060 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE bar 0x0
# R_CAPABILITY_FOR_ALL-NEXT:   0x20000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# R_CAPABILITY_FOR_ALL-NEXT:   0x20040 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# R_CAPABILITY_FOR_ALL-NEXT: }
# R_CAPABILITY_FOR_ALL-NEXT: DynamicSymbols [
# R_CAPABILITY_FOR_ALL-NEXT:   Symbol {
# R_CAPABILITY_FOR_ALL-NEXT:     Name:  (0)
# R_CAPABILITY_FOR_ALL-NEXT:     Value: 0x0
# R_CAPABILITY_FOR_ALL-NEXT:     Size: 0
# R_CAPABILITY_FOR_ALL-NEXT:     Binding: Local
# R_CAPABILITY_FOR_ALL-NEXT:     Type: None (0x0)
# R_CAPABILITY_FOR_ALL-NEXT:     Other: 0
# R_CAPABILITY_FOR_ALL-NEXT:     Section: Undefined (0x0)
# R_CAPABILITY_FOR_ALL-NEXT:   }
# R_CAPABILITY_FOR_ALL-NEXT:   Symbol {
# R_CAPABILITY_FOR_ALL-NEXT:     Name: bar (1)
# R_CAPABILITY_FOR_ALL-NEXT:     Value: 0x10040
# R_CAPABILITY_FOR_ALL-NEXT:     Size: 8
# R_CAPABILITY_FOR_ALL-NEXT:     Binding: Local
# R_CAPABILITY_FOR_ALL-NEXT:     Type: Function (0x2)
# R_CAPABILITY_FOR_ALL-NEXT:     Other [
# R_CAPABILITY_FOR_ALL-NEXT:       STV_INTERNAL
# R_CAPABILITY_FOR_ALL-NEXT:     ]
# R_CAPABILITY_FOR_ALL-NEXT:     Section: .text (0x8)
# R_CAPABILITY_FOR_ALL-NEXT:  }
# R_CAPABILITY_FOR_ALL-NEXT:   Symbol {
# R_CAPABILITY_FOR_ALL-NEXT:     Name: foo (5)
# R_CAPABILITY_FOR_ALL-NEXT:     Value: 0x10000
# R_CAPABILITY_FOR_ALL-NEXT:     Size: 4
# R_CAPABILITY_FOR_ALL-NEXT:     Binding: Global
# R_CAPABILITY_FOR_ALL-NEXT:     Type: Function (0x2)
# R_CAPABILITY_FOR_ALL-NEXT:     Other: 0
# R_CAPABILITY_FOR_ALL-NEXT:     Section: .text (0x8)
# R_CAPABILITY_FOR_ALL-NEXT:   }
# R_CAPABILITY_FOR_ALL-NEXT: ]

# DUMP-R_CAPABILITY_FOR_ALL: CAPABILITY RELOCATION RECORDS:
# DUMP-R_CAPABILITY_FOR_ALL-EMPTY:
# DUMP-R_CAPABILITY_FOR_ALL-NEXT: Contents of section .data:
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20000 00000000 00000000 cacacaca cacacaca
#                                             ^---- addend 0 for preemptible_ref
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20010 11111111 11111111 11111111 11111111
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20020 00000000 00000000 cacacaca cacacaca
#                                             ^---- addend 0 for local_ref
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20030 13131313 13131313 13131313 13131313
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20040 00000000 00000002 cacacaca cacacaca
#                                             ^---- addend 0x2 for preemptible_ref_with_addend
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20050 13131313 13131313 13131313 13131313
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20060 00000000 00000004 cacacaca cacacaca
#                                             ^---- addend 0x4 for local_ref_with_addend
# DUMP-R_CAPABILITY_FOR_ALL-NEXT:  20070 14141414 14141414 14141414 14141414
