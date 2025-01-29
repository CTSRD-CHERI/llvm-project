# REQUIRES: riscv
# RUN: llvm-mc -filetype=obj -triple=riscv64-unknown-elf --mattr=+zcheripurecap,+cap-mode --target-abi=l64pc128 %s -o %t.rv32.o
# RUN: ld.lld --local-caprelocs=cbuildcap %t.rv32.o -o %t.rv32

# RUN: llvm-readobj --relocs %t.rv32 | FileCheck %s --check-prefix=RELOCS
# RUN: llvm-objdump -s --section=.captable %t.rv32 | FileCheck %s --check-prefix=CAPTAB
# RUN: llvm-readobj --symbols %t.rv32 | FileCheck %s --check-prefix=SYM

# RELOCS:      Relocations [
# RELOCS-NEXT:   Section ({{[0-9]+}}) .rela.dyn {
# RELOCS-NEXT:     0x12240 R_RISCV_CHERI_RELATIVE - 0x0
# RELOCS-NEXT:   }

# CAPTAB: Contents of section .captable:
# CAPTAB-NEXT: 12240 50320100 00000000 50329504 0070e201
#                    [    address    ] [      meta     ]

# SYM:        Name: __rela_dyn_start 
# SYM:        Name: __rela_dyn_end
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
        auipc   ca0, %captab_pcrel_hi(x)
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
