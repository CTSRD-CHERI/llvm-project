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
# CHECK-NEXT: [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# CHECK:      [10] .eh_frame         PROGBITS        0000000000020468 000468 00003c 00  WA  0   0  8


# CHECK-LABEL: Relocation section '.rel.dyn'
# CHECK-SAME:  at offset 0x{{.+}} contains 1 entries:
# CHECK-NEXT:    Offset             Info             Type               Symbol's Value  Symbol's Name
# CHECK-NEXT: 0000000000020488  0000000000001203 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE

# CHECK-LABEL: There are 11 program headers, starting at offset 64
# CHECK:       Program Headers:
# CHECK-NEXT:   Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:   PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x000268 0x000268 R   0x8
# CHECK-NEXT:   LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x00045c 0x00045c R   0x10000
# CHECK-NEXT:   LOAD           0x000460 0x0000000000010460 0x0000000000010460 0x000004 0x000004 R E 0x10000
# CHECK-NEXT:   LOAD           0x000468 0x0000000000020468 0x0000000000020468 0x00003c 0x00003c RW  0x10000
# CHECK-NEXT:   LOAD           0x0004b0 0x00000000000304b0 0x00000000000304b0 0x000010 0x000010 RW  0x10000
# CHECK-NEXT:   DYNAMIC        0x000330 0x0000000000000330 0x0000000000000330 0x000100 0x000100 R   0x8
# CHECK-NEXT:   GNU_RELRO      0x000468 0x0000000000020468 0x0000000000020468 0x00003c 0x000b98 R   0x1
# CHECK-NEXT:   GNU_EH_FRAME   0x000448 0x0000000000000448 0x0000000000000448 0x000014 0x000014 R   0x4
# CHECK-NEXT:   GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x0
# CHECK-NEXT:   OPTIONS        0x0002c0 0x00000000000002c0 0x00000000000002c0 0x000028 0x000028 R   0x8
# CHECK-NEXT:   ABIFLAGS       0x0002a8 0x00000000000002a8 0x00000000000002a8 0x000018 0x000018 R   0x8
# CHECK-EMPTY:
#
# CHECK-LABEL: Section to Segment mapping:
# CHECK-NEXT:  Segment Sections...
# CHECK-NEXT:   00
# CHECK-NEXT:   01     .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr .rel.dyn .eh_frame_hdr
# CHECK-NEXT:   02     .text
# CHECK-NEXT:   03     .eh_frame
# CHECK-NEXT:   04     .data .got
# CHECK-NEXT:   05     .dynamic
# CHECK-NEXT:   06     .eh_frame
# CHECK-NEXT:   07     .eh_frame_hdr
# CHECK-NEXT:   08


