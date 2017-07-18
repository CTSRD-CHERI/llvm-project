# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-linux %s -o %t-linux.o
# RUN: ld.lld -r -o %t-linux-r.o %t-linux.o
# RUN: llvm-readobj -r %t-linux-r.o | FileCheck -check-prefix RELOCATABLE %s
# RUN: ld.lld -shared -o %t-linux.so %t-linux.o
# RUN: llvm-readobj -r %t-linux.so | FileCheck -check-prefixes SHARED,SHARED-LINUX %s

# Same now for freebsd where we change input RELA to output REL
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-freebsd.o
# RUN: ld.lld -r -o %t-freebsd-r.o %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd-r.o | FileCheck -check-prefix RELOCATABLE %s
# RUN: ld.lld -shared -o %t-freebsd.so %t-freebsd.o
# RUNTODO: llvm-readobj -r %t-freebsd.so | FileCheck -check-prefixes SHARED,SHARED-FREEBSD %s

.extern foo

.data
.quad foo + 0x10
.quad 0xabcdef0012345678

# SHARED:       Relocations [
# SHARED-FREEBSD-NEXT: Section (7) .rel.dyn {
# SHARED-LINUX-NEXT:   Section (7) .rela.dyn {
# SHARED-NEXT:           0x10000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo 0x10
# SHARED-NEXT:         }
# SHARED-NEXT: ]

# RELOCATABLE:     Relocations [
# RELOCATABLE-NEXT:   Section (3) .rela.data {
# RELOCATABLE-NEXT:    0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE foo 0x10
# RELOCATABLE-NEXT:  }
# RELOCATABLE-NEXT:]
