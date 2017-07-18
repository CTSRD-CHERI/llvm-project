# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-linux %s -o %t-linux.o
# RUN: llvm-objdump --section=.data -s %t-linux.o | FileCheck -check-prefix DATA-RELA %s

# RUN: ld.lld -r -o %t-linux-r.o %t-linux.o
# RUN: llvm-readobj -r %t-linux-r.o | FileCheck -check-prefix RELOCATABLE %s

# RUN: ld.lld -shared -o %t-linux.so %t-linux.o
# RUN: llvm-readobj -r %t-linux.so | FileCheck -check-prefixes SHARED,SHARED-LINUX %s
# RUN: llvm-objdump --section=.data -s %t-linux.so | FileCheck -check-prefix DATA-RELA %s

# 32 bits
# RUN: llvm-mc -filetype=obj -triple=mips-unknown-freebsd %s -o %t-freebsd32.o
# RUN: llvm-objdump --section=.data -s %t-freebsd32.o | FileCheck -check-prefix DATA-REL %s

# RUN: ld.lld -r -o %t-freebsd32-r.o %t-freebsd32.o
# RUN: llvm-objdump --section=.data -s %t-freebsd32-r.o | FileCheck -check-prefix DATA-REL %s
# RUN: llvm-readobj -r %t-freebsd32-r.o | FileCheck -check-prefix RELOCATABLE-REL %s

# RUN: ld.lld -shared -o %t-freebsd32.so %t-freebsd32.o
# RUN: llvm-readobj -r %t-freebsd32.so | FileCheck -check-prefixes SHARED,SHARED-FREEBSD32 %s
# RUN: llvm-objdump --section=.data -s %t-freebsd32.so | FileCheck -check-prefix DATA-REL %s

# Same now for freebsd MIPS 64 bits where we change input RELA to output REL
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-freebsd.o
# RUN: llvm-objdump --section=.data -s %t-freebsd.o | FileCheck -check-prefix DATA-RELA %s

# RUN: ld.lld -r -o %t-freebsd-r.o %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd-r.o | FileCheck -check-prefix RELOCATABLE %s
# RUN: llvm-objdump --section=.data -s %t-freebsd-r.o | FileCheck -check-prefix DATA-RELA %s

# RUN: ld.lld -shared -o %t-freebsd.so %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd.so | FileCheck -check-prefixes SHARED,SHARED-FREEBSD %s
# RUN: llvm-objdump --section=.data -s %t-freebsd.so | FileCheck -check-prefix DATA-REL %s


.extern foo

.data
.quad foo + 0x10
.quad 0xabcdef0012345678

# SHARED:       Relocations [
# SHARED-FREEBSD-NEXT: Section (7) .rel.dyn {
# SHARED-FREEBSD-NEXT:         0x10000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
# SHARED-FREEBSD32-NEXT: Section (7) .rel.dyn {
# SHARED-FREEBSD32-NEXT:       0x10000 R_MIPS_REL32 foo 0x0 (real addend unknown)
# SHARED-LINUX-NEXT:   Section (7) .rela.dyn {
# SHARED-LINUX-NEXT:           0x10000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo 0x10
# SHARED-NEXT:         }
# SHARED-NEXT: ]

# RELOCATABLE:     Relocations [
# RELOCATABLE-NEXT:   Section (3) .rela.data {
# RELOCATABLE-NEXT:    0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE foo 0x10
# RELOCATABLE-NEXT:  }
# RELOCATABLE-NEXT:]

# RELOCATABLE-REL:     Relocations [
# RELOCATABLE-REL-NEXT:   Section (3) .rel.data {
# FIXME: 0x10 is in the location of the relocation but llvm-readobj doesn't print it....
# RELOCATABLE-REL-NEXT:    0x0 R_MIPS_64 foo 0x0 (real addend unknown)
# RELOCATABLE-REL-NEXT:  }
# RELOCATABLE-REL-NEXT:]


# DATA-RELA:      Contents of section .data:
# DATA-RELA-NEXT: 0000 00000000 00000000 abcdef00 12345678  .............4Vx

# Rel should have the relocation value 10 in .data
# DATA-REL:      Contents of section .data:
# DATA-REL-NEXT: 0000 00000000 00000010 abcdef00 12345678  .............4Vx
