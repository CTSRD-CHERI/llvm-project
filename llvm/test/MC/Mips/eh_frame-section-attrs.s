# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-readobj -r -s %t.o | FileCheck %s -check-prefix OBJECT
# RUN: llvm-mc -filetype=obj -position-independent -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-readobj -r -s %t.o | FileCheck %s -check-prefixes OBJECT,PIC


# OBJECT-LABEL:  Section {
# OBJECT:         Name: .eh_frame
# OBJECT-NEXT:    Type: SHT_PROGBITS (0x1)
# OBJECT-NEXT:    Flags [
# OBJECT-NEXT:      SHF_ALLOC (0x2)
# For PIC we need to also emit SHF_WRITE since the relocation needs to be processed at runtime
# PIC-NEXT:         SHF_WRITE (0x1)
# OBJECT-NEXT:    ]
# OBJECT-NEXT:    Address: 0x0
# OBJECT-NEXT:    Offset: 0x90
# OBJECT-NEXT:    Size: 48
# OBJECT-NEXT:    Link: 0
# OBJECT-NEXT:    Info: 0
# OBJECT-NEXT:    AddressAlignment: 8
# OBJECT-NEXT:    EntrySize: 0
# OBJECT-NEXT:  }

# OBJECT-LABEL: Relocations [
# OBJECT-NEXT:   Section ({{.+}}) .rela.eh_frame {
# OBJECT-NEXT:     0x1C R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .text 0x0
# OBJECT-NEXT:   }
# OBJECT-NEXT: ]

func:
.cfi_startproc
lui	$3, 1
.cfi_endproc
