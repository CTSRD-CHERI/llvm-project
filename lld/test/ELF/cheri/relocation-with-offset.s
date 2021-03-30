# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-linux %s -o %t-linux.o
# RUN: llvm-objdump --section=.data -s %t-linux.o | FileCheck --check-prefix DATA-RELA %s

# RUN: ld.lld -r -o %t-linux-r.o %t-linux.o
# RUN: llvm-readobj -r %t-linux-r.o | FileCheck --check-prefix RELOCATABLE %s

# RUN: ld.lld -shared -o %t-linux.so %t-linux.o
# RUN: llvm-readobj -r %t-linux.so | FileCheck --check-prefixes SHARED,SHARED-LINUX %s
# RUN: llvm-objdump --section=.data -s %t-linux.so | FileCheck --check-prefix DATA-REL %s

# 32 bits
# RUN: llvm-mc -filetype=obj -triple=mips-unknown-freebsd %s -o %t-freebsd32.o -defsym=MIPS32=1
# RUN: llvm-objdump --section=.data -s %t-freebsd32.o | FileCheck --check-prefix DATA-REL %s

# RUN: ld.lld -r -o %t-freebsd32-r.o %t-freebsd32.o
# RUN: llvm-objdump --section=.data -s %t-freebsd32-r.o | FileCheck --check-prefix DATA-REL %s
# RUN: llvm-readobj -r %t-freebsd32-r.o | FileCheck --check-prefix RELOCATABLE-MIPS32 %s

# RUN: ld.lld -shared -o %t-freebsd32.so %t-freebsd32.o
# RUN: llvm-readobj -r %t-freebsd32.so | FileCheck --check-prefixes SHARED,SHARED-FREEBSD32 %s
# RUN: llvm-objdump --section=.data -s %t-freebsd32.so | FileCheck --check-prefix DATA-REL %s

# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-freebsd.o
# RUN: llvm-objdump --section=.data -s %t-freebsd.o | FileCheck --check-prefix DATA-RELA %s

# RUN: ld.lld -r -o %t-freebsd-r.o %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd-r.o | FileCheck --check-prefix RELOCATABLE %s
# RUN: llvm-objdump --section=.data -s %t-freebsd-r.o | FileCheck --check-prefix DATA-RELA %s

# RUN: ld.lld -shared -o %t-freebsd.so %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd.so | FileCheck --check-prefixes SHARED,SHARED-FREEBSD %s
# RUN: llvm-objdump --section=.data -s %t-freebsd.so | FileCheck --check-prefix DATA-REL %s


.extern foo

.data
.ifdef MIPS32
.word 0
.word foo + 0x10
.else
.quad foo + 0x10
.endif
.quad 0xabcdef0012345678

# SHARED:       Relocations [
# SHARED-FREEBSD-NEXT: Section (7) .rel.dyn {
# SHARED-FREEBSD-NEXT:         0x203A0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo{{$}}
# SHARED-FREEBSD32-NEXT: Section (7) .rel.dyn {
# SHARED-FREEBSD32-NEXT:       0x20234 R_MIPS_REL32 foo{{$}}
# SHARED-LINUX-NEXT:   Section (7) .rel.dyn {
# SHARED-LINUX-NEXT:           0x203A0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo{{$}}
# SHARED-NEXT:         }
# SHARED-NEXT: ]

# RELOCATABLE:     Relocations [
# RELOCATABLE-NEXT:   Section ({{.+}}) .rela.data {
# RELOCATABLE-NEXT:    0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE foo 0x10
# RELOCATABLE-NEXT:  }
# RELOCATABLE-NEXT:]

# RELOCATABLE-MIPS32:     Relocations [
# RELOCATABLE-MIPS32-NEXT:   Section ({{.+}}) .rel.data {
# FIXME: 0x10 is in the location of the relocation but llvm-readobj doesn't print it....
# RELOCATABLE-MIPS32-NEXT:    0x4 R_MIPS_32 foo{{$}}
# RELOCATABLE-MIPS32-NEXT:  }
# RELOCATABLE-MIPS32-NEXT:]


# DATA-RELA:      Contents of section .data:
# DATA-RELA-NEXT: {{[0-9a-f]+}} 00000000 00000000 abcdef00 12345678  .............4Vx

# Rel should have the relocation value 10 in .data
# DATA-REL:      Contents of section .data:
# DATA-REL-NEXT: {{[0-9a-f]+}} 00000000 00000010 abcdef00 12345678  .............4Vx
