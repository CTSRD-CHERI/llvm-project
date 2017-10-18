# RUN: llvm-mc -triple mips64-unknown-freebsd -filetype=obj -o %t.o %s
# RUN: llvm-mc -triple mips64-unknown-freebsd -show-encoding %s
# RUN: llvm-objdump -r -d %t.o
# RUN: llvm-objdump -r -d %t.o | FileCheck %s
# RUN: llvm-readobj -r %t.o

# FIXME: llvm-mc -triple mips64-unknown-freebsd -show-encoding -position-independent %s
# FIXME: we shouldn't be generating %lo and %hi relocations for pic!
# FIXME: should this be an error?

# CHECK-NOT: R_MIPS_32

# We were generating extra R_MIPS_32 relocations on the sw instruction in locore.S which then resulted in an unknown instruction trap!

 .data

.globl fenvp
fenvp:
.space 4 # Assumes mips32? Is that OK?


.set noreorder
.text

.ent _locore
.globl _locore
_locore:
sw $6, fenvp
nop
lw $6, fenvp
nop

nop
nop

sd $6, fenvp
nop
ld $6, fenvp
nop


.globl _locoreEnd
_locoreEnd:
.end _locore
