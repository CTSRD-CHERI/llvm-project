# RUN: llvm-mc -triple mips64-unknown-freebsd -mattr=+%cheri_type,+chericap -filetype=obj %s -o - \
# RUN:     | llvm-readobj -r - -s -section-data | FileCheck %s -check-prefixes CHECK,%cheri_type
# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o - | llvm-readobj -r - -s -section-data | FileCheck %s -check-prefixes CHECK,%cheri_type

.extern foo
.data
.chericap foo
.chericap foo+1
.chericap foo-1

# CHECK:      Section {
# CHECK:         Name: .data
# CHECK:         SectionData (
# CHECK-NEXT:      0000: CACACACA CACACACA CACACACA CACACACA  |................|
# CHECK-NEXT:      0010: CACACACA CACACACA CACACACA CACACACA  |................|
# CHECK-NEXT:      0020: CACACACA CACACACA CACACACA CACACACA  |................|
# CHERI256-NEXT:   0030: CACACACA CACACACA CACACACA CACACACA  |................|
# CHERI256-NEXT:   0040: CACACACA CACACACA CACACACA CACACACA  |................|
# CHERI256-NEXT:   0050: CACACACA CACACACA CACACACA CACACACA  |................|
# CHECK-NEXT:    )

.rodata
.chericap 0x0
.chericap 0x12345678abcdef00

# CHECK:      Section {
# CHECK:         Name: .rodata
# CHECK:         SectionData (
# CHERI128-NEXT:      0000: 00000000 00000000 00000000 00000000  |................|
# CHERI128-NEXT:      0010: 00000000 00000000 12345678 ABCDEF00  |.........4Vx....|

# CHERI256-NEXT:      0000: 00000000 00000000 00000000 00000000  |................|
# CHERI256-NEXT:      0010: 00000000 00000000 00000000 00000000  |................|
# CHERI256-NEXT:      0020: 00000000 00000000 12345678 ABCDEF00  |.........4Vx....|
# CHERI256-NEXT:      0030: 00000000 00000000 00000000 00000000  |................|
# CHECK-NEXT:    )

# CHECK-LABEL: Relocations [
# CHECK-NEXT:   Section ({{.+}}) .rela.data {
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x1
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0xFFFFFFFFFFFFFFFF
# CHECK-NEXT:   }
# CHECK-NEXT: ]
