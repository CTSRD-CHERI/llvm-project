# REQUIRES: riscv
# RUN: llvm-mc -filetype=obj -triple=riscv64-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=l64pc128 %s -o %t.rv64.o
# RUN: ld.lld --local-caprelocs=elf %t.rv64.o -o %t.rv64

# RUN: llvm-readobj --relocs %t.rv64 | FileCheck %s --check-prefix=RELOCS
# RUN: llvm-objdump -s --section=.rela.dyn %t.rv64 | FileCheck %s --check-prefix=RELADYN
# RUN: llvm-objdump -s --section=.data %t.rv64 | FileCheck %s --check-prefix=DATA
# RUN: llvm-objdump -s --section=.rodata %t.rv64 | FileCheck %s --check-prefix=RODATA
# RUN: llvm-readobj --cap-relocs-cbuildcap %t.rv64 | FileCheck %s --check-prefix=CAPRELOCS
# RUN: llvm-readelf -s %t.rv64 | FileCheck %s --check-prefix=SYM

# RELOCS:      Relocations [
# RELOCS-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS-NEXT:     0x121C0 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:     0x121D0 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:     0x121E0 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:   }

# RELADYN: Contents of section .rela.dyn:
# RELADYN-NEXT: 10158
# RELADYN-NEXT: 10168
# RELADYN-NEXT: 10178
# RELADYN-NEXT: 10188
# RELADYN-NEXT: 10198

# DATA: Contents of section .data:
# DATA-NEXT: 121c0   ac110100 00000000 ac116e0c 00d8ef01
#                    address = 0x111ac -> matches symbol VA (_start)
# DATA-NEXT: 121d0   f0210100 00000000 f0217d04 0078ee01
#                    address = 0x121f0 -> matches symbol VA (x)
# DATA-NEXT: 121e0   a4010100 00000000 a4016a04 0058ee01
#                    address = 0x101a4 -> matches symbol VA (y)
#                    [    address    ] [      meta     ]

# RODATA: Contents of section .rodata:

#CAPRELOCS: CHERI __rela_dyn relocs [
#CAPRELOCS-NEXT: 0x0121c0 (foo) Base: 0x111ac (_start+0) Length: 12 Perms: Function
#CAPRELOCS-NEXT: 0x0121d0 Base: 0x121f0 (x+0) Length: 4 Perms: Object
#CAPRELOCS-NEXT: 0x0121e0 Base: 0x101a4 (y+0) Length: 4 Perms: Constant
#CAPRELOCS-NEXT: ]

# SYM:      0000000000010158    72 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_start
# SYM-NEXT: 00000000000101a0    0  NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_end
# SYM:      00000000000111ac    12 FUNC    GLOBAL DEFAULT     3 _start
# SYM:      00000000000121f0    4  OBJECT  GLOBAL DEFAULT     4 x
# SYM:      00000000000101a4    4  OBJECT  GLOBAL DEFAULT     2 y


# RUN: llvm-mc -filetype=obj -triple=riscv32-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=il32pc64 %s -o %t.rv32.o
# RUN: ld.lld --local-caprelocs=elf %t.rv32.o -o %t.rv32

# RUN: llvm-readobj --relocs %t.rv32 | FileCheck %s --check-prefix=RELOCS32
# RUN: llvm-objdump -s --section=.rela.dyn %t.rv32 | FileCheck %s --check-prefix=RELADYN32
# RUN: llvm-objdump -s --section=.data %t.rv32 | FileCheck %s --check-prefix=DATA32
# RUN: llvm-readobj --cap-relocs-cbuildcap %t.rv32 | FileCheck %s --check-prefix=CAPRELOCS32
# RUN: llvm-readelf -s %t.rv32 | FileCheck %s --check-prefix=SYM32

# RELOCS32:      Relocations [
# RELOCS32-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS32-NEXT:     0x12110 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS32-NEXT:     0x12118 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS32-NEXT:     0x12120 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS32-NEXT:   }

#RELADYN32: Contents of section .rela.dyn:
#RELADYN32-NEXT: 100d4
#RELADYN32-NEXT: 100e4
#RELADYN32-NEXT: 100f4

# DATA32: Contents of section .data:
# DATA32-NEXT: 12110  04110100 044118d1 28210100 28b108fd
#                     [ addr ] [ meta ] [ addr ] [ meta ]
#                     address = 0x11104 -> matches symbol VA (_start)
#                     address = 0x12128 -> matches symbol VA (x)
#
# DATA32-NEXT: 12120  fc000100 fc0008f7
#                     [ addr ] [ meta ]
#                     meta = 0xf60800fc
#                     address = 0x100fc -> matches symbol VA (y)

#CAPRELOCS32: CHERI __rela_dyn relocs [
#CAPRELOCS32: 0x012110 (foo) Base: 0x11104 (_start+0) Length: 12 Perms: Function
#CAPRELOCS32: 0x012118 Base: 0x12128 (x+0) Length: 4 Perms: Object
#CAPRELOCS32: 0x012120 Base: 0x100fc (y+0) Length: 4 Perms: Constant
#CAPRELOCS32-NEXT: ]

# SYM32:      000100d4    36 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_start
# SYM32-NEXT: 000100f8    0  NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_end
# SYM32:      00011104    12 FUNC    GLOBAL DEFAULT     3 _start
# SYM32:      00012128    4  OBJECT  GLOBAL DEFAULT     4 x
# SYM32:      000100fc    4  OBJECT  GLOBAL DEFAULT     2 y

.global _start, x, y, foo, bar
.data
.type foo,@object
foo:
  ## create a function relocation
  .chericap _start
  ## create a object relocation
  .chericap x
  ## const reloc
  .chericap y
.size foo, . - foo

.type x,@object
x:
  .word 3
  .size x, 4

.rodata
  .word 0
.type y,@object
y:
  .word 7
  .size y, 4

.text
.type bar,@function
bar:
  nop
.size bar, . - bar

.type _start,@function
_start:
  nop
  nop
  nop
.size _start, . - _start

.weak __rela_dyn_start
.hidden __rela_dyn_start
.weak __rela_dyn_end
.hidden __rela_dyn_end
