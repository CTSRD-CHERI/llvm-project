# REQUIRES: mips
# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: ld.lld -z now -shared %t.o -o %t.so
# RUN: llvm-readelf --program-headers --section-mapping %t.so | FileCheck %s -check-prefix SEGMENTS

# SEGMENTS-LABEL: Section to Segment mapping:
# SEGMENTS-NEXT:  Segment Sections...
# SEGMENTS-NEXT:   00
# __cap_relocs should be in the rodata segment (since it uses relative relocations)
# SEGMENTS-NEXT:   01     .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr .rodata __cap_relocs
# SEGMENTS-NEXT:   02     .data .got
# SEGMENTS-NEXT:   03     .dynamic

# RUN: llvm-readobj --sections --dynamic-table %t.so | FileCheck %s

.rodata
.protected foo
.type foo,@object
foo:
.space 16
.size foo, 16

.data
.chericap foo + 1

# CHECK-LABEL: Sections [
# CHECK:      Name: __cap_relocs
# CHECK-NEXT: Type: SHT_PROGBITS (0x1)
# CHECK-NEXT: Flags [ (0x2)
# CHECK-NEXT:   SHF_ALLOC (0x2)
# CHECK-NEXT: ]
# CHECK-NEXT: Address: [[CAPRELOCS_ADDR:0x3A0]]
# CHECK-NEXT: Offset: [[CAPRELOCS_ADDR]]
# CHECK-NEXT: Size: 40
# CHECK-NEXT: Link: 0
# CHECK-NEXT: Info: 0
# CHECK-NEXT: AddressAlignment: 8
# CHECK-NEXT: EntrySize: 40
# CHECK-NEXT: }

# CHECK-LABEL: DynamicSection [ (18 entries)
# CHECK:      0x000000000000001E FLAGS BIND_NOW
# CHECK:      0x000000006FFFFFFB FLAGS_1 NOW
# CHECK:      0x000000007000C002 MIPS_CHERI_FLAGS ABI_PCREL RELATIVE_CAPRELOCS
# flag name is so long that there is no space:
# CHECK-NEXT: 0x000000007000C000 MIPS_CHERI___CAPRELOCS [[CAPRELOCS_ADDR]]
# CHECK-NEXT: 0x000000007000C001 MIPS_CHERI___CAPRELOCSSZ 0x28
