# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: ld.lld -shared %t.o -o %t.so
# RUN: llvm-readobj -sections -dynamic-table %t.so | FileCheck %s


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
# CHECK-NEXT: Flags [ (0x3)
# CHECK-NEXT:   SHF_ALLOC (0x2)
# CHECK-NEXT:   SHF_WRITE (0x1)
# CHECK-NEXT: ]
# CHECK-NEXT: Address: 0x20000
# CHECK-NEXT: Offset: 0x20000
# CHECK-NEXT: Size: 40
# CHECK-NEXT: Link: 0
# CHECK-NEXT: Info: 0
# CHECK-NEXT: AddressAlignment: 8
# CHECK-NEXT: EntrySize: 40
# CHECK-NEXT: }

# CHECK-LABEL: DynamicSection [ (18 entries)
# CHECK:  0x000000007000C000 CHERI___CAPRELOCS    0x20000
# CHECK:  0x000000007000C001 CHERI___CAPRELOCSSZ  0x28
