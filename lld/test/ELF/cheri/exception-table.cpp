// RUN: %cheri_purecap_cc1 -O2 -mframe-pointer=none -fcxx-exceptions -fexceptions %s -emit-obj -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=MIPS-OBJ-RELOCS
// RUN: %riscv64_cheri_purecap_cc1 -O2 -mframe-pointer=none -fcxx-exceptions -fexceptions %s -emit-obj -o %t-riscv64.o
// RUN: llvm-readobj -r %t-riscv64.o | FileCheck %s --check-prefix=RV64-OBJ-RELOCS
/// Should have two relocations against a local alias for _Z4testll
// MIPS-OBJ-RELOCS:      Section ({{.+}}) .rela.gcc_except_table {
// MIPS-OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L_Z4testll$local
// MIPS-OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L_Z4testll$local
// MIPS-OBJ-RELOCS-NEXT: }
// RV64-OBJ-RELOCS:      Section ({{.+}}) .rela.gcc_except_table {
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY .L_Z4testll$local
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY .L_Z4testll$local
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_ADD32 - 0x0
// RV64-OBJ-RELOCS-NEXT:   R_RISCV_SUB32 - 0x0
// RV64-OBJ-RELOCS-NEXT: }

/// This should work with both -z text and -z notext
/// Check that .gcc_except_table ends up in the relro section and the relocations are correct
// RUN: ld.lld -shared %t.o -o %t.so -z notext
// RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s --check-prefixes=HEADERS,MIPS-RELOCS
// RUN: ld.lld -shared %t.o -o %t.so -z text
// RUN: llvm-readelf -r --section-mapping %t.so
// RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s --check-prefixes=HEADERS,MIPS-RELOCS

// HEADERS-LABEL: There are 10 program headers, starting at
// HEADERS-EMPTY:
// HEADERS-NEXT: Program Headers:
// HEADERS-NEXT: Type           Offset   VirtAddr PhysAddr FileSiz  MemSiz   Flg Align
// HEADERS-NEXT: PHDR           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x10000
// HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R E 0x10000
// HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
// HEADERS-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
// HEADERS-NEXT: DYNAMIC        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// HEADERS-NEXT: GNU_RELRO      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x1
// HEADERS-NEXT: GNU_STACK      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x0
// HEADERS-NEXT: OPTIONS        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// HEADERS-NEXT: ABIFLAGS       0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// HEADERS-EMPTY:
// HEADERS-NEXT: Section to Segment mapping:
// HEADERS-NEXT: Segment Sections...
// HEADERS-NEXT: 00
// HEADERS-NEXT: 01     .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr .rel.dyn .rel.plt .eh_frame __cap_relocs {{$}}
// HEADERS-NEXT: 02     .text
// HEADERS-NEXT: 03     .gcc_except_table
// HEADERS-NEXT: 04     .data .captable .got
// HEADERS-NEXT: 05     .dynamic
// HEADERS-NEXT: 06     .gcc_except_table
// HEADERS-NEXT: 07
// HEADERS-NEXT: 08     .MIPS.options
// HEADERS-NEXT: 09     .MIPS.abiflags
// HEADERS-NEXT: None   .bss .mdebug.abi64 .pdr .comment .symtab .shstrtab .strtab

// MIPS-RELOCS-LABEL:      Relocation section '.rel.dyn' {{.+}} contains 1 entries:
// MIPS-RELOCS-NEXT: Offset             Info             Type                            Symbol's Value  Symbol's Name
// MIPS-RELOCS-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __gxx_personality_v0
// MIPS-RELOCS-EMPTY:
// MIPS-RELOCS-NEXT: Relocation section '.rel.plt' at offset {{.+}} contains 3 entries:
// MIPS-RELOCS-NEXT: Offset             Info             Type                            Symbol's Value   Symbol's Name
// MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 _Z11external_fnl
// MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_begin_catch
// MIPS-RELOCS-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_end_catch

/// Local relocations for exception handling:
// MIPS-RELOCS-NEXT: CHERI __cap_relocs [
// MIPS-RELOCS-NEXT:   0x02{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+132) Length: 164 Perms: Function
// MIPS-RELOCS-NEXT:   0x02{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+100) Length: 164 Perms: Function
// MIPS-RELOCS-NEXT: ]

/// Should also emit __cap_relocs for RISC-V:
// RUN: ld.lld -shared %t-riscv64.o -o %t.so
// RUN: llvm-readelf -r --cap-relocs %t.so | FileCheck %s --check-prefixes=RV64-RELOCS
// RV64-RELOCS: CHERI __cap_relocs [
// RV64-RELOCS-NEXT:   0x002{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+96) Length: 120 Perms: Function
// RV64-RELOCS-NEXT:   0x002{{.+}} Base: 0x1{{.+}} (.L_Z4testll$local+72) Length: 120 Perms: Function
// RV64-RELOCS-NEXT: ]

long external_fn(long arg);

long test(long arg, long arg2) {
  int a = 0;
  int b = 0;
  try {
    a = external_fn(arg);
  } catch (...) {
    return -1;
  }
  try {
    b = external_fn(arg2);
  } catch (...) {
    return a;
  }
  return a + b;
}
