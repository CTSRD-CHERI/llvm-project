# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --compartment-policy=%t/compartments.json %t/one.o -o %t/one
# RUN: llvm-readelf -l %t/one | FileCheck %s

## Verify that .data.rel.ro is treated as relro even with a compartment suffix.
## Must be covered by a GNU_RELRO and included in CHERI_PCC.

# CHECK-LABEL: Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:  PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x0001c0 0x0001c0 R   0x8
# CHECK-NEXT:  LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x000205 0x000205 R   0x1000
# CHECK-NEXT:  LOAD           0x000208 0x0000000000011208 0x0000000000011208 0x000008 0x000008 R E 0x1000
# CHECK-NEXT:  LOAD           0x000210 0x0000000000012210 0x0000000000012210 0x000008 0x000008 RW  0x1000
# CHECK-NEXT:  C18N_NAME      0x000208 0x0000000000011208 0x0000000000000001 0x000010 0x001010     0x4
# CHECK-NEXT:      [Compartment: one]
# CHECK-NEXT:  GNU_RELRO      0x000210 0x0000000000012210 0x0000000000012210 0x000008 0x000df0 R   0x1
# CHECK-NEXT:  CHERI_PCC      0x000208 0x0000000000011208 0x0000000000011208 0x000010 0x001010 R E 0x8
# CHECK-NEXT:  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0
# CHECK-EMPTY:
# CHECK-NEXT: Section to Segment mapping:
# CHECK-NEXT:  Segment Sections...
# CHECK-NEXT:   00
# CHECK-NEXT:   01     .c18nstrtab
# CHECK-NEXT:   02     .text.one
# CHECK-NEXT:   03     .data.rel.ro.one .pad.cheri.pcc.one
# CHECK-NEXT:   04     .text.one .data.rel.ro.one .pad.cheri.pcc.one
# CHECK-NEXT:   05     .data.rel.ro.one .pad.cheri.pcc.one
# CHECK-NEXT:   06     .text.one .data.rel.ro.one .pad.cheri.pcc.one
# CHECK-NEXT:   07
# CHECK-NEXT:   None   .comment .symtab .shstrtab .strtab


#--- one.s

cllc ca0, foo

.data.rel.ro
.type foo, %object
foo: .byte 42
.size foo, . - foo

#--- compartments.json

{
  "compartments": {
    "one": {
      "files": ["one.o"]
    }
  }
}
