# RUN: llvm-mc -triple mips64-unknown-freebsd -mattr=+cheri128,+chericap -filetype=obj %s -o - \
# RUN:     | llvm-readobj -r - -s -section-data
# RUN: llvm-mc -triple mips64-unknown-freebsd -mattr=+cheri128,+chericap -filetype=obj %s -o - \
# RUN:     | llvm-readobj -r - -s -section-data | %cheri_FileCheck %s -check-prefixes CHECK
# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o - | llvm-readobj -r - -s -section-data | %cheri_FileCheck %s -check-prefixes CHECK

.extern foo
.data
.chericap foo
.chericap foo+1
## Check that we pad sensibly
.byte 1
.chericap foo-1

# CHECK:      Section {
# CHECK:         Name: .data
# CHECK-NEXT:    Type: SHT_PROGBITS (0x1)
# CHECK-NEXT:    Flags [ (0x3)
# CHECK-NEXT:      SHF_ALLOC (0x2)
# CHECK-NEXT:      SHF_WRITE (0x1)
# CHECK-NEXT:    ]
# CHECK-NEXT:    Address: 0x0
# CHECK-NEXT:    Offset: 0x
# CHECK-NEXT:    Size: [[#CAP_SIZE * 4]]
# CHECK-NEXT:    Link: 0
# CHECK-NEXT:    Info: 0
# CHECK-NEXT:    AddressAlignment: [[#CAP_SIZE]]
# CHECK-NEXT:    EntrySize: 0
# CHECK-NEXT:    SectionData (
# CHECK-NEXT:      0000: CACACACA CACACACA CACACACA CACACACA |
# CHECK-NEXT:      0010: CACACACA CACACACA CACACACA CACACACA |
# CHECK-NEXT:      0020: 01000000 00000000 00000000 00000000 |
# CHECK-NEXT:      0030: CACACACA CACACACA CACACACA CACACACA |
# CHECK-NEXT:    )

.rodata
.chericap 0x0
.byte 0xaa
.chericap 0x12345678abcdef00

# CHECK:      Section {
# CHECK:         Name: .rodata
# CHECK-NEXT:    Type: SHT_PROGBITS (0x1)
# CHECK-NEXT:    Flags [ (0x2)
# CHECK-NEXT:      SHF_ALLOC (0x2)
# CHECK-NEXT:    ]
# CHECK-NEXT:    Address: 0x0
# CHECK-NEXT:    Offset:
# CHECK-NEXT:    Size: [[#CAP_SIZE * 3]]
# CHECK-NEXT:    Link: 0
# CHECK-NEXT:    Info: 0
# CHECK-NEXT:    AddressAlignment: [[#CAP_SIZE]]
# CHECK-NEXT:    EntrySize: 0
# CHECK-NEXT:    SectionData (
# CHECK-NEXT:      0000: 00000000 00000000 00000000 00000000  |
# CHECK-NEXT:      0010: AA000000 00000000 00000000 00000000  |
# CHECK-NEXT:      0020: 00000000 00000000 12345678 ABCDEF00  |
# CHECK-NEXT:    )

# CHECK-LABEL: Relocations [
# CHECK-NEXT:   Section ({{.+}}) .rela.data {
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x1
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0xFFFFFFFFFFFFFFFF
# CHECK-NEXT:   }
# CHECK-NEXT: ]
