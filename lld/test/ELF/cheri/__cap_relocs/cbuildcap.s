# REQUIRES: riscv
# RUN: llvm-mc -filetype=obj -triple=riscv64-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=l64pc128 %s -o %t.rv64.o
# RUN: ld.lld --local-caprelocs=cbuildcap %t.rv64.o -o %t.rv64

# RUN: llvm-readobj --relocs %t.rv64 | FileCheck %s --check-prefix=RELOCS
# RUN: llvm-objdump -s --section=.rela.dyn %t.rv64 | FileCheck %s --check-prefix=RELADYN
# RUN: llvm-objdump -s --section=.got %t.rv64 | FileCheck %s --check-prefix=GOT
# RUN: llvm-readobj --cap-relocs-cbuildcap %t.rv64 | FileCheck %s --check-prefix=CAPRELOCS
# RUN: llvm-readelf -s %t.rv64 | FileCheck %s --check-prefix=SYM

# RELOCS:      Relocations [
# RELOCS-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS-NEXT:     0x12250 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:   }

#RELADYN: Contents of section .rela.dyn:
#RELADYN-NEXT: 10200 50220100 00000000 ca000000 00000000

# GOT: Contents of section .got:
# GOT-NEXT: 12240 
# GOT-NEXT: 12250  60320100 00000000 60329904 0070e201
#                  [    address    ] [      meta     ]

#CAPRELOCS: CHERI __rela_dyn relocs [
#CAPRELOCS: 0x012250 Base: 0x13260 (x+0) Length: 4 Perms: Object
#CAPRELOCS-NEXT: ]

# SYM:      0000000000010200   24 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_start
# SYM-NEXT: 0000000000010218     0 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_end
# SYM:      0000000000013260     4 OBJECT  GLOBAL DEFAULT     4 x


# RUN: llvm-mc -filetype=obj -triple=riscv32-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=il32pc64 %s -o %t.rv32.o
# RUN: ld.lld --local-caprelocs=cbuildcap %t.rv32.o -o %t.rv32

# RUN: llvm-readobj --relocs %t.rv32 | FileCheck %s --check-prefix=RELOCS32
# RUN: llvm-objdump -s --section=.rela.dyn %t.rv32 | FileCheck %s --check-prefix=RELADYN32
# RUN: llvm-objdump -s --section=.got %t.rv32 | FileCheck %s --check-prefix=GOT32
# RUN: llvm-readobj --cap-relocs-cbuildcap %t.rv32 | FileCheck %s --check-prefix=CAPRELOCS32
# RUN: llvm-readelf -s %t.rv32 | FileCheck %s --check-prefix=SYM32

# RELOCS32:      Relocations [
# RELOCS32-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS32-NEXT:     0x12168 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS32-NEXT:   }

#RELADYN32: Contents of section .rela.dyn:
#RELADYN32-NEXT: 10134 68210100 ca000000 00000000

# GOT32: Contents of section .got:
# GOT32-NEXT: 12160 00000000 00000000 70310100 70d109fe
#                   [address] [ meta ]
#                    address = 0x12160 -> matches symbol VA

#CAPRELOCS32: CHERI __rela_dyn relocs [
#CAPRELOCS32: 0x012168 Base: 0x13170 (x+0) Length: 4 Perms: Object
#CAPRELOCS32-NEXT: ]

# SYM32:      00010134    12 NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_start
# SYM32-NEXT: 00010140    0  NOTYPE  LOCAL  HIDDEN      1 __rela_dyn_end
# SYM32:      00013170    4  OBJECT  GLOBAL DEFAULT     4 x

        .text
        .attribute      4, 16
        .attribute      5, "rv64i2p1_zcheripurecap0p9"
        .file   "test.c"
        .globl  foo                             # -- Begin function foo
        .p2align        2
        .type   foo,@function
foo:                                    # @foo
.Lfoo$local:
        .type   .Lfoo$local,@function
# %bb.0:                                # %entry
        caddi   csp, csp, -16
.LBB0_1:                                # %entry
                                        # Label of block must be emitted
        auipc   ca0, %got_pcrel_hi(x)
        lc      ca0, %pcrel_lo(.LBB0_1)(ca0)
        sc      ca0, 0(csp)
        lc      ca0, 0(csp)
        caddi   csp, csp, 16
        ret
.Lfunc_end0:
        .size   foo, .Lfunc_end0-foo
        .size   .Lfoo$local, .Lfunc_end0-foo
                                        # -- End function
        .type   x,@object                       # @x
        .bss
        .globl  x
        .p2align        2, 0x0
x:
        .word   0                               # 0x0
        .size   x, 4

        .ident  "clang version 17.0.0 (git@gitlab.codasip.com:cheri/software/bakewell/cherillvm.git f885005c7c20325bf3300c4fe2e28e8aa57d34b3)"
        .section        ".note.GNU-stack","",@progbits

.global _start 
_start:
   call foo 
.weak __rela_dyn_start
.hidden __rela_dyn_start
.weak __rela_dyn_end
.hidden __rela_dyn_end
