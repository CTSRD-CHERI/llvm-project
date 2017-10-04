# MIPS TLS variables that are marked as local by a version script were previously
# writing values to the GOT that causes runtime crashes
# This was happending when linking jemalloc_tsd.c in FreeBSD libc
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: echo "{ global: foo; local: *; };" > %t.script
# RUN: ld.lld --version-script %t.script -shared %t.o -o %t.so
# RUN: llvm-objdump --section=.got -s %t.so | FileCheck %s -check-prefix GOT
# RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix RELOCS

# GOT:        Contents of section .got:
# GOT-NEXT:   20000 00000000 00000000 80000000 00000000
# GOT-NEXT:   20010 00000000 00000000 00000000 00000000
# GOT-NEXT:   20020 ffffffff ffff8000

# RELOCS:      Section ({{.+}}) .rel.dyn {
# RELOCS-NEXT:  0x20018 R_MIPS_TLS_DTPMOD64/R_MIPS_NONE/R_MIPS_NONE
# RELOCS-NEXT: }

# Test case generated using clang -mcpu=mips4 -target mips64-unknown-freebsd12.0 -fpic -O -G0 -EB -mabi=n64 -msoft-float -std=gnu99 -S %s -o %t.s
# from the following source:
#
# _Thread_local int x;
# int foo() { return x; }
#
        .text
        .abicalls
        .section        .mdebug.abi64,"",@progbits
        .nan    legacy
        .file   "mips64-thread-relocation.c"
        .text
        .globl  foo
        .p2align        3
        .type   foo,@function
        .set    nomicromips
        .set    nomips16
        .ent    foo
foo:
        .frame  $fp,32,$ra
        .mask   0x00000000,0
        .fmask  0x00000000,0
        .set    noreorder
        .set    nomacro
        .set    noat
        daddiu  $sp, $sp, -32
        sd      $ra, 24($sp)
        sd      $fp, 16($sp)
        sd      $gp, 8($sp)
        move     $fp, $sp
        lui     $1, %hi(%neg(%gp_rel(foo)))
        daddu   $1, $1, $25
        daddiu  $gp, $1, %lo(%neg(%gp_rel(foo)))
        ld      $25, %call16(__tls_get_addr)($gp)
        jalr    $25
        daddiu  $4, $gp, %tlsgd(x)
        lw      $2, 0($2)
        move     $sp, $fp
        ld      $gp, 8($sp)
        ld      $fp, 16($sp)
        ld      $ra, 24($sp)
        jr      $ra
        daddiu  $sp, $sp, 32
        .set    at
        .set    macro
        .set    reorder
        .end    foo
.Lfunc_end0:
        .size   foo, .Lfunc_end0-foo

        .type   x,@object
        .section        .tbss,"awT",@nobits
        .globl  x
        .p2align        2
x:
        .4byte  0
        .size   x, 4


