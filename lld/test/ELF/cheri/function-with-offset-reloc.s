## Check that LLD emits a warning when emitting a relocation against function+offset
## This may no longer work in the future since it's not ideal when using sentries
## and tightly bounded code capabilities.

# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=OBJ
# RUN: ld.lld %t.o -shared -o %t.so 2>&1 | FileCheck %s --check-prefix=WARNING-MSG
# RUN: llvm-readobj -r -sections --section-data %t.so | FileCheck %s

# WARNING-MSG: warning: got capability relocation with non-zero addend (0x8) against function func. This may not be supported by the runtime linker.
# WARNING-MSG-NEXT: >>> defined in {{.+}}function-with-offset-reloc.s.tmp.o
# WARNING-MSG-NEXT: >>> referenced by{{.+}}function-with-offset-reloc.s.tmp.o:(.data+0x10)

.type func,@function
.global func

.data
bad_reloc:
.byte 0x1
.chericap func + 0x8

# OBJ-LABEL: Relocations [
# OBJ-NEXT:   Section ({{.+}}) .rela.data {
# OBJ-NEXT:     0x10 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE func 0x8
# OBJ-NEXT:   }
# OBJ-NEXT: ]

# CHECK:   Section {
# CHECK:      Index: 9
# CHECK:      Name: .data
# CHECK-NEXT: Type: SHT_PROGBITS (0x1)
# CHECK-NEXT: Flags [ (0x3)
# CHECK-NEXT: SHF_ALLOC (0x2)
# CHECK-NEXT: SHF_WRITE (0x1)
# CHECK-NEXT: ]
# CHECK-NEXT: Address: 0x203B0
# CHECK-NEXT: Offset: 0x3B0
# CHECK-NEXT: Size: 32
# CHECK-NEXT: Link: 0
# CHECK-NEXT: Info: 0
# CHECK-NEXT: AddressAlignment: 16
# CHECK-NEXT: EntrySize: 0
# CHECK-NEXT: SectionData (
# CHECK-NEXT:   0000: 01000000 00000000 00000000 00000000
# CHECK-NEXT:   0010: 00000000 00000008 CACACACA CACACACA
##                                   ^^ Addend  8
# CHECK-NEXT: )

# CHECK-LABEL:  Relocations [
# CHECK-NEXT:    Section ({{.+}}) .rel.dyn {
# CHECK-NEXT:      R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE func 0x0 (real addend unknown)
# CHECK-NEXT:    }
# CHECK-NEXT:  ]
