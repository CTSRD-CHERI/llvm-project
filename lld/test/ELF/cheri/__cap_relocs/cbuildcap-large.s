# REQUIRES: riscv
# RUN: llvm-mc -filetype=obj -triple=riscv64-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=l64pc128 %s -o %t.rv32.o
# RUN: ld.lld --local-caprelocs=elf %t.rv32.o -o %t.rv32

# RUN: llvm-readobj --relocs %t.rv32 | FileCheck %s --check-prefix=RELOCS
# RUN: llvm-objdump -s --section=.rela.dyn %t.rv32 | FileCheck %s --check-prefix=RELADYN
# RUN: llvm-objdump -s --section=.got %t.rv32 | FileCheck %s --check-prefix=GOT
# RUN: llvm-readelf -s %t.rv32 | FileCheck %s --check-prefix=SYM

# RELOCS:      Relocations [
# RELOCS-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS-NEXT:     0x12200 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:   }

#RELADYN: Contents of section .rela.dyn:
#RELADYN-NEXT: 101c8
#RELADYN-NEXT: 101d8 00000000 00000000

# GOT: Contents of section .got:
# GOT-NEXT: 121f0
# GOT-NEXT: 12200 10320100 00000000 0b994502 0078ee01
#                 [    address    ] [      meta     ]
#                 address = 0x13250 -> matches symbol VA

# SYM:      00000000000101c8    24 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_start
# SYM-NEXT: 00000000000101e0     0 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_end
# SYM:      0000000000013210  8193 OBJECT  GLOBAL DEFAULT     4 x
.text
.globl  x, _start
_start:
  auipc ca0, %got_pcrel_hi(x)
  lc ca0, %pcrel_lo(_start)(ca0)
.size _start, . - _start

.data
  .type x, @object
x:
  .word   0
  .size   x, 0x2001

.weak __rela_dyn_start
.hidden __rela_dyn_start
.weak __rela_dyn_end
.hidden __rela_dyn_end
