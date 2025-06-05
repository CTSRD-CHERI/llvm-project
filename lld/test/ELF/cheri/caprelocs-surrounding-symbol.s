## Check that lld uses the size of surrounding symbols for capability relocations
## against STT_NONE targets with size==0.
# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: llvm-readobj --symbols --relocations --cap-relocs --expand-relocs %t.o | FileCheck %s --check-prefix=OBJ-RELOCS
# RUN: ld.lld -pie %t.o -o %t.exe -e fn1 --verbose 2>&1 | FileCheck %s --check-prefix=LLD-OUTPUT "--implicit-check-not=warning:"
# RUN: llvm-readobj --symbols --relocations --cap-relocs --expand-relocs %t.exe | FileCheck %s --check-prefix=EXE-RELOCS
# LLD-OUTPUT: relaxation passes: 1

.text
.global fn1
fn1:
.type fn1, @function
# .Lfn1$start is not referenced so can be discarded by the assembler.
.Lfn1$start:
.type .Lfn1$start, @function
  nop
  nop
.Lfirst_fn_target:
  cret
.size .Lfn1$start, . - .Lfn1$start
.size fn1, . - .Lfn1$start

# Same as fn1 but this time with a local fn2$start symbol that is retained in the symbol table.
.global fn2
fn2:
.type fn2, @function
.Lfn2$start:
.type .Lfn2$start, @function
  nop
  nop
.Lsecond_fn_target:
  cret
.size fn2, . - .Lfn2$start
.size .Lfn2$start, . - .Lfn2$start


.data
obj1:
.type obj1, @object
.Lobj1_begin:
.space 16
.Linside_obj1:  # a relocation against this label should pick the surrounding object
.space 16
.size obj1, . - .Lobj1_begin

# This one should have multiple matches - prefer the smallest non-zero-size surrounding object.
obj2:
.type obj2, @object
.Lobj2_begin:
.space 16
.type obj2_subobject1, @object
obj2_subobject1: # this label has a type, but a zero size. Don't attempt to find a better match.
.space 1
.type obj2_subobject2, @object
obj2_subobject2:
.space 1
.Linside_obj2_subobject2:
.space 14
.size obj2_subobject2, . - obj2_subobject2
.size obj2, . - .Lobj2_begin



# OBJ-RELOCS-LABEL: Relocations [
# OBJ-RELOCS-NEXT:   Section ({{.+}}) .rela.relocs_section {
# EXE-RELOCS-LABEL:  CHERI __cap_relocs [
.section .relocs_section, "aw"
fn_reloc1:
.chericap .Lfirst_fn_target
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x0
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: .Lfirst_fn_target (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# LLD-OUTPUT-NEXT: Found better match for capability relocation against .Lfirst_fn_target+0: fn1+8
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x3480 (fn_reloc1)
# EXE-RELOCS-NEXT:     Base: fn1 (0x1370)
# EXE-RELOCS-NEXT:     Offset: 8
# EXE-RELOCS-NEXT:     Length: 24
# EXE-RELOCS-NEXT:     Permissions: Function (0x8000000000000000)
# EXE-RELOCS-NEXT:   }

fn_reloc2:
# Ensure that the local symbol .Lfn2$start is not discarded by the assembler
.reloc ., R_RISCV_NONE, .Lfn2$start
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x10
# OBJ-RELOCS-NEXT:     Type: R_RISCV_NONE (0)
# OBJ-RELOCS-NEXT:     Symbol: .Lfn2$start (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
.chericap .Lsecond_fn_target
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x10
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: .Lsecond_fn_target (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# LLD-OUTPUT-NEXT: Found better match for capability relocation against .Lsecond_fn_target+0: .Lfn2$start+8
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x3490 (fn_reloc2)
# EXE-RELOCS-NEXT:     Base: fn1 (0x1370)
# EXE-RELOCS-NEXT:     Offset: 20
# EXE-RELOCS-NEXT:     Length: 24
# EXE-RELOCS-NEXT:     Permissions: Function (0x8000000000000000)
# EXE-RELOCS-NEXT:   }
data_reloc1:
.chericap .Linside_obj1
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x20
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: .Linside_obj1 (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# LLD-OUTPUT-NEXT: Found better match for capability relocation against .Linside_obj1+0: obj1+16
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x34A0 (data_reloc1)
# EXE-RELOCS-NEXT:     Base: obj1 (0x3438)
# EXE-RELOCS-NEXT:     Offset: 16
# EXE-RELOCS-NEXT:     Length: 32
# EXE-RELOCS-NEXT:     Permissions: Object (0x0)
# EXE-RELOCS-NEXT:   }
data_reloc2:
.chericap obj2_subobject1
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x30
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: obj2_subobject1 (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# LLD-OUTPUT-NEXT: warning: could not determine size of cap reloc against local object obj2_subobject1
# LLD-OUTPUT-NEXT: >>> defined in {{.+}}caprelocs-surrounding-symbol.s.tmp.o:(obj2))
# LLD-OUTPUT-NEXT: >>> referenced by local <unknown kind> data_reloc2
# LLD-OUTPUT-NEXT: >>> defined in {{.+}}caprelocs-surrounding-symbol.s.tmp.o:(.relocs_section+0x30))
# LLD-OUTPUT-EMPTY:
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x34B0 (data_reloc2)
# EXE-RELOCS-NEXT:     Base: obj2_subobject1 (0x3468)
# EXE-RELOCS-NEXT:     Offset: 0
# EXE-RELOCS-NEXT:     Length: 16
# EXE-RELOCS-NEXT:     Permissions: Object (0x0)
# EXE-RELOCS-NEXT:   }
data_reloc3:
.chericap obj2_subobject2
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x40
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: obj2_subobject2 (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x34C0 (data_reloc3)
# EXE-RELOCS-NEXT:     Base: obj2_subobject2 (0x3469)
# EXE-RELOCS-NEXT:     Offset: 0
# EXE-RELOCS-NEXT:     Length: 15
# EXE-RELOCS-NEXT:     Permissions: Object (0x0)
# EXE-RELOCS-NEXT:   }
data_reloc4:
.chericap .Linside_obj2_subobject2
# OBJ-RELOCS-NEXT:   Relocation {
# OBJ-RELOCS-NEXT:     Offset: 0x50
# OBJ-RELOCS-NEXT:     Type: R_RISCV_CHERI_CAPABILITY (193)
# OBJ-RELOCS-NEXT:     Symbol: .Linside_obj2_subobject2 (
# OBJ-RELOCS-NEXT:     Addend: 0x0
# OBJ-RELOCS-NEXT:   }
# LLD-OUTPUT-NEXT: Found better match for capability relocation against .Linside_obj2_subobject2+0: obj2+18
# LLD-OUTPUT-NEXT: Found better match for capability relocation against obj2+18: obj2_subobject2+1
# EXE-RELOCS-NEXT:   Relocation {
# EXE-RELOCS-NEXT:     Location: 0x34D0 (data_reloc4)
# EXE-RELOCS-NEXT:     Base: obj2_subobject2 (0x3469)
# EXE-RELOCS-NEXT:     Offset: 1
# EXE-RELOCS-NEXT:     Length: 15
# EXE-RELOCS-NEXT:     Permissions: Object (0x0)
# EXE-RELOCS-NEXT:   }

## Check there are no further relocation
# OBJ-RELOCS-NEXT:  }
# OBJ-RELOCS-NEXT: ]
# EXE-RELOCS-NEXT: ]
# LLD-OUTPUT-EMPTY:
