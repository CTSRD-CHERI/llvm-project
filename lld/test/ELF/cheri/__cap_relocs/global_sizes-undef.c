// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-obj %s -o %t.o
// RUN: ld.lld -shared -o %t.so %t.o -verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix LINKER-MSG
// LINKER-MSG: warning: .global_sizes section contains unresolved values -> making writable because it references unresolved symbol errno
// LINKER-MSG-NEXT: warning: Could not find .global_size for <undefined> (in GOT) errno
// LINKER-MSG-NEXT: Writing size 0x0 for <undefined> (in GOT) errno

// RUN: llvm-readelf -r -program-headers %t.so | FileCheck %s -check-prefix UNDEFINED
// RUN: llvm-objdump -s --section=.global_sizes %t.so | FileCheck %s -check-prefix DUMP

// Now try with a defined errno variable:
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-obj %s -o %t.o -DDEFINED_ERRNO=1
// RUN: ld.lld -shared -o %t.so %t.o -verbose-cap-relocs
// RUN: llvm-readelf -r -program-headers %t.so
// RUN: llvm-readelf -r -program-headers %t.so | FileCheck %s -check-prefix DEFINED-MAPPING
// RUN: llvm-objdump -s --section=.global_sizes %t.so | FileCheck %s -check-prefix DEFINED-DUMP

#ifndef DEFINED_ERRNO
extern int errno;
#else
int errno;
#endif

int* __error() {
  return &errno;
}

// And that .global_sizes is writable because we still need to fill in the correct value
// UNDEFINED-LABEL: Section to Segment mapping:
// UNDEFINED-NEXT:  Segment Sections...
// UNDEFINED-NEXT:   00
// UNDEFINED-NEXT:   01     .dynsym .dynstr .MIPS.abiflags .MIPS.options .hash .dynamic
// UNDEFINED-NEXT:   02     .text
// UNDEFINED-NEXT:   03     .global_sizes .data .got
// UNDEFINED-NEXT:   04     .dynamic
// UNDEFINED-NEXT:   05


// Check that we filled in zeroes in the .global_sizes sections:
// DUMP:      Contents of section .global_sizes:
// DUMP-NEXT: 20000 00000000 00000000                    ........

// DEFINED-MAPPING-LABEL: Section to Segment mapping:
// DEFINED-MAPPING-NEXT:  Segment Sections...
// DEFINED-MAPPING-NEXT:   00
// DEFINED-MAPPING-NEXT:   01     .dynsym .dynstr .global_sizes .MIPS.abiflags .MIPS.options .hash .dynamic
// DEFINED-MAPPING-NEXT:   02     .text
// DEFINED-MAPPING-NEXT:   03     .data .got
// DEFINED-MAPPING-NEXT:   04     .dynamic
// DEFINED-MAPPING-NEXT:   05

// DEFINED-DUMP:      Contents of section .global_sizes:
// DEFINED-DUMP-NEXT: 0210 00000000 00000004                    ........
