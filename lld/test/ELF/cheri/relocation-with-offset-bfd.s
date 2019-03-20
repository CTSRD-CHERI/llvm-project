# REQUIRES: cheri_ld_bfd

# Try freebsd mips 64
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t-freebsd.o
# RUN: llvm-objdump --section=.data -s %t-freebsd.o | FileCheck -check-prefix DATA-RELA %s

# RUN: %cheri_bfd -r -o %t-freebsd-r.o %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd-r.o | FileCheck -check-prefix RELOCATABLE %s
# RUN: llvm-objdump --section=.data -s %t-freebsd-r.o | FileCheck -check-prefix DATA-RELA %s

# RUN: %cheri_bfd -shared -o %t-freebsd.so %t-freebsd.o
# RUN: llvm-readobj -r %t-freebsd.so | FileCheck -check-prefixes SHARED %s
# RUN: llvm-objdump --section=.data -s %t-freebsd.so | FileCheck -check-prefix DATA-REL %s


.extern foo

.data
.quad foo + 0x10
.quad 0xabcdef0012345678

# SHARED:       Relocations [
# SHARED-NEXT: Section ({{.+}}) .rel.dyn {
# SHARED-NEXT:         0x0 R_MIPS_NONE/R_MIPS_NONE/R_MIPS_NONE - 0x0 (real addend unknown)
# SHARED-NEXT:         0x10490 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
# SHARED-NEXT:    }
# SHARED-NEXT: ]

# RELOCATABLE:     Relocations [
# RELOCATABLE-NEXT:   Section ({{.+}}) .rela.data {
# RELOCATABLE-NEXT:    0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE foo 0x10
# RELOCATABLE-NEXT:  }
# RELOCATABLE-NEXT:]

# RELOCATABLE-REL:     Relocations [
# RELOCATABLE-REL-NEXT:   Section ({{.+}}) .rel.data {
# FIXME: 0x10 is in the location of the relocation but llvm-readobj doesn't print it....
# RELOCATABLE-REL-NEXT:    0x0 R_MIPS_64 foo 0x0 (real addend unknown)
# RELOCATABLE-REL-NEXT:  }
# RELOCATABLE-REL-NEXT:]


# DATA-RELA:      Contents of section .data:
# DATA-RELA-NEXT: 0000 00000000 00000000 abcdef00 12345678  .............4Vx

# Rel should have the relocation value 10 in .data
# DATA-REL:      Contents of section .data:
# DATA-REL-NEXT: 10490 00000000 00000010 abcdef00 12345678  .............4Vx
