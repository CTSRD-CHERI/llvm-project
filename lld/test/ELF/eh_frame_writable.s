# RUN: llvm-mc -filetype=obj -position-independent -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-readobj -r -s %t.o | FileCheck %s -check-prefix OBJECT
# RUN: ld.lld --eh-frame-hdr --shared --fatal-warnings -o %t.so %t.o
# RUN: llvm-readelf -program-headers -relocations -sections %t.so | FileCheck %s

# OBJECT-LABEL:  Section {
# OBJECT:         Name: .eh_frame
# OBJECT-NEXT:    Type: SHT_PROGBITS (0x1)
# OBJECT-NEXT:    Flags [ (0x3)
# OBJECT-NEXT:      SHF_ALLOC (0x2)
# OBJECT-NEXT:      SHF_WRITE (0x1)
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

.global foo
foo:
.cfi_startproc
lui	$11, 1
.cfi_endproc


# CHECK-LABEL: Section Headers:
# CHECK: [12] .eh_frame         PROGBITS        0000000000030000 030000 00003c 00  WA  0   0  8


# CHECK-LABEL: Relocation section '.rel.dyn' at offset 0x390 contains 1 entries:
# CHECK-NEXT:    Offset             Info             Type               Symbol's Value  Symbol's Name
# CHECK-NEXT: 0000000000030020  0000000000001203 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE

# CHECK-LABEL: There are 8 program headers, starting at offset 64
# CHECK-LABEL: Program Headers:
# CHECK-NEXT:   Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:   PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x0001c0 0x0001c0 R   0x8
# CHECK-NEXT:   LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x0003b4 0x0003b4 R   0x10000
# CHECK-NEXT:   LOAD           0x010000 0x0000000000010000 0x0000000000010000 0x000004 0x000004 R E 0x10000
# CHECK-NEXT:   LOAD           0x020000 0x0000000000020000 0x0000000000020000 0x01003c 0x020000 RW  0x10000
# CHECK-NEXT:   DYNAMIC        0x000290 0x0000000000000290 0x0000000000000290 0x000100 0x000100 R   0x8
# CHECK-NEXT:   GNU_RELRO      0x030000 0x0000000000030000 0x0000000000030000 0x00003c 0x001000 R   0x1
# CHECK-NEXT:   GNU_EH_FRAME   0x0003a0 0x00000000000003a0 0x00000000000003a0 0x000014 0x000014 R   0x4
# CHECK-NEXT:   GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0

# CHECK-LABEL: Section to Segment mapping:
# CHECK-NEXT:  Segment Sections...
# CHECK-NEXT:   00
# CHECK-NEXT:   01     .dynsym .dynstr .MIPS.abiflags .MIPS.options .hash .dynamic .rel.dyn .eh_frame_hdr
# CHECK-NEXT:   02     .text
# CHECK-NEXT:   03     .data .got .eh_frame
# CHECK-NEXT:   04     .dynamic
# CHECK-NEXT:   05     .eh_frame
# CHECK-NEXT:   06     .eh_frame_hdr
# CHECK-NEXT:   07
