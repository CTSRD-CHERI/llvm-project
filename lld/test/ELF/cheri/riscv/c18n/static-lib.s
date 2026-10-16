# REQUIRES: riscv

# RUN: rm -rf %t && split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/a.s -o %t/a.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/b.s -o %t/b.o
# RUN: llvm-ar crs %t/libb.a %t/b.o
# RUN: ld.lld -o %t/fully-scoped %t/a.o %t/libb.a \
# RUN:   --compartment-policy=%t/fully-scoped.json
# RUN: ld.lld -o %t/scope-only %t/a.o %t/libb.a \
# RUN:   --compartment-policy=%t/scope-only.json
# RUN: not ld.lld -o %t/unscoped %t/a.o %t/libb.a \
# RUN:   --compartment-policy=%t/unscoped.json 2>&1 | \
# RUN:	 FileCheck %s --check-prefix=UNSCOPED
# RUN: llvm-readelf -l %t/fully-scoped | FileCheck %s --check-prefix=SCOPED
# RUN: llvm-readelf -l %t/scope-only | FileCheck %s --check-prefix=SCOPED

# SCOPED-LABEL: Program Headers:
# SCOPED-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# SCOPED-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x000150 0x000150 R   0x8
# SCOPED-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x000195 0x000195 R   0x1000
# SCOPED-NEXT:    LOAD           0x000198 0x0000000000011198 0x0000000000011198 0x00000c 0x00000c R E 0x1000
# SCOPED-NEXT:    C18N_NAME      0x000198 0x0000000000011198 0x0000000000000001 0x00000c 0x00000c     0x4
# SCOPED-NEXT:        [Compartment: one]
# SCOPED-NEXT:    CHERI_PCC      0x000198 0x0000000000011198 0x0000000000011198 0x00000c 0x00000c R E 0x4
# SCOPED-NEXT:    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# SCOPED-LABEL:  Section to Segment mapping:
# SCOPED-NEXT:    Segment Sections...
# SCOPED-NEXT:     00
# SCOPED-NEXT:     01     .c18nstrtab
# SCOPED-NEXT:     02     .text.one
# SCOPED-NEXT:     03     .text.one
# SCOPED-NEXT:     04     .text.one
# SCOPED-NEXT:     05
# SCOPED-NEXT:     None   .pad.cheri.pcc.one .comment .symtab .shstrtab .strtab

# UNSCOPED: error: no output sections for compartment one

#--- a.s

call foo

#--- b.s

.global foo
.type foo, %function
foo:
  ret

#--- fully-scoped.json

{
  "compartments": {
    "one": {
      "files": ["libb.a:b.o"]
    }
  }
}

#--- scope-only.json

{
  "compartments": {
    "one": {
      "files": ["libb.a"]
    }
  }
}

#--- unscoped.json

{
  "compartments": {
    "one": {
      "files": ["b.o"]
    }
  }
}
