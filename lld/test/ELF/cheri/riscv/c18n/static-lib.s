# REQUIRES: riscv

# RUN: rm -rf %t && split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/a.s -o %t/a.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/b.s -o %t/b.o
# RUN: llvm-ar crs %t/libb.a %t/b.o
# RUN: ld.lld -o %t/scoped %t/a.o %t/libb.a \
# RUN:   --compartment-policy=%t/scoped.json \
# RUN:   --compartment-policy=%t/unscoped.json
# RUN: ld.lld -o %t/unscoped %t/a.o %t/libb.a \
# RUN:   --compartment-policy=%t/unscoped.json
# RUN: llvm-readelf -l %t/scoped | FileCheck %s --check-prefix=SCOPED
# RUN: llvm-readelf -l %t/unscoped | FileCheck %s --check-prefix=UNSCOPED

# SCOPED-LABEL: Program Headers:
# SCOPED-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# SCOPED-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x0001f8 0x0001f8 R   0x8
# SCOPED-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x000278 0x000278 R   0x1000
# SCOPED-NEXT:    LOAD           0x000278 0x0000000000011278 0x0000000000011278 0x000028 0x000028 R E 0x1000
# SCOPED-NEXT:    LOAD           0x0002a0 0x00000000000122a0 0x00000000000122a0 0x000010 0x000010 RW  0x1000
# SCOPED-NEXT:    C18N_NAME      0x000278 0x0000000000011278 0x0000000000000001 0x000004 0x000004     0x4
# SCOPED-NEXT:        [Compartment: scoped]
# SCOPED-NEXT:    C18N_NAME      0x000280 0x0000000000011280 0x0000000000000008 0x000030 0x001030     0x10
# SCOPED-NEXT:        [Compartment: unscoped]
# SCOPED-NEXT:    CHERI_PCC      0x000278 0x0000000000011278 0x0000000000011278 0x000004 0x000004 R E 0x4
# SCOPED-NEXT:    CHERI_PCC      0x000280 0x0000000000011280 0x0000000000011280 0x000030 0x001030 R E 0x10
# SCOPED-NEXT:    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# SCOPED-LABEL:  Section to Segment mapping:
# SCOPED-NEXT:    Segment Sections...
# SCOPED-NEXT:     00
# SCOPED-NEXT:     01     .c18nstrtab __cap_relocs
# SCOPED-NEXT:     02     .text.scoped .pad.cheri.pcc.scoped .text.unscoped .iplt.unscoped
# SCOPED-NEXT:     03     .got.plt.unscoped
# SCOPED-NEXT:     04     .text.scoped
# SCOPED-NEXT:     05     .text.unscoped .iplt.unscoped .got.plt.unscoped
# SCOPED-NEXT:     06     .text.scoped
# SCOPED-NEXT:     07     .text.unscoped .iplt.unscoped .got.plt.unscoped
# SCOPED-NEXT:     08
# SCOPED-NEXT:     None   .pad.cheri.pcc.unscoped .comment .symtab .shstrtab .strtab

# UNSCOPED-LABEL: Program Headers:
# UNSCOPED-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# UNSCOPED-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x000150 0x000150 R   0x8
# UNSCOPED-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x00019a 0x00019a R   0x1000
# UNSCOPED-NEXT:    LOAD           0x00019c 0x000000000001119c 0x000000000001119c 0x00000c 0x00000c R E 0x1000
# UNSCOPED-NEXT:    C18N_NAME      0x00019c 0x000000000001119c 0x0000000000000001 0x00000c 0x00000c     0x4
# UNSCOPED-NEXT:        [Compartment: unscoped]
# UNSCOPED-NEXT:    CHERI_PCC      0x00019c 0x000000000001119c 0x000000000001119c 0x00000c 0x00000c R E 0x4
# UNSCOPED-NEXT:    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# UNSCOPED-LABEL:  Section to Segment mapping:
# UNSCOPED-NEXT:    Segment Sections...
# UNSCOPED-NEXT:     00
# UNSCOPED-NEXT:     01     .c18nstrtab
# UNSCOPED-NEXT:     02     .text.unscoped
# UNSCOPED-NEXT:     03     .text.unscoped
# UNSCOPED-NEXT:     04     .text.unscoped
# UNSCOPED-NEXT:     05
# UNSCOPED-NEXT:     None   .pad.cheri.pcc.unscoped .comment .symtab .shstrtab .strtab

#--- a.s

call foo

#--- b.s

.global foo
.type foo, %function
foo:
  ret

#--- scoped.json

{
  "compartments": {
    "scoped": {
      "files": ["libb.a:b.o"]
    }
  }
}

#--- unscoped.json

{
  "compartments": {
    "unscoped": {
      "files": ["a.o", "b.o"]
    }
  }
}
