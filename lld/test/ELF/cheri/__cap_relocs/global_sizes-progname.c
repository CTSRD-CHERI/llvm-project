// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-obj %s -o %t.o
// RUN: ld.lld --fatal-warnings -shared -o %t.so %t.o -verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix LINKER-MSG
// LINKER-MSG: Using .global_size for symbol __progname in shared lib (assuming size==sizeof(void* __capability)
// LINKER-MSG-NEXT: Writing size 0x{{1|2}}0 for <undefined> (in GOT) __progname
// LINKER-MSG-NEXT: Using .global_size for symbol environ in shared lib (assuming size==sizeof(void* __capability)
// LINKER-MSG-NEXT: Writing size 0x{{1|2}}0 for <undefined> (in GOT) environ

// RUN: llvm-readelf -r -program-headers %t.so | FileCheck %s -check-prefix PHDRS
// RUN: llvm-objdump -s --section=.global_sizes %t.so | FileCheck %s -check-prefix DUMP

extern char* __progname;
extern char** environ;

char* getprogname() {
  return __progname;
}

char* getenv(void) {
  return environ[0];
}


// .global_sizes should be read-only:
// PHDRS-LABEL: Section to Segment mapping:
// PHDRS-NEXT:  Segment Sections...
// PHDRS-NEXT:   00
// PHDRS-NEXT:   01     .dynsym .dynstr .global_sizes .MIPS.abiflags .MIPS.options .hash .dynamic
// PHDRS-NEXT:   02     .text
// PHDRS-NEXT:   03     .data .got
// PHDRS-NEXT:   04     .dynamic
// PHDRS-NEXT:   05

// DUMP:      Contents of section .global_sizes:
// DUMP-NEXT: 00000000 000000{{1|2}}0 00000000 000000{{1|2}}0
