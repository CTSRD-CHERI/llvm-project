# RUN: llvm-mc -triple mips64-unknown-freebsd -mattr=+%cheri_type,+chericap -filetype=obj %s -o - | llvm-readobj -r - | FileCheck %s
# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o - | llvm-readobj -r - | FileCheck %s

.extern foo

.chericap foo
.chericap foo+1
.chericap foo-1

# TODO: support .chericap 0x12345 to simplify writing of untagged values

# CHECK: Relocations [
# CHECK-NEXT:   Section (3) .rela.text {
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x1
# CHECK-NEXT:     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0xFFFFFFFFFFFFFFFF
# CHECK-NEXT:   }
# CHECK-NEXT: ]
