// RUN: %cheri_purecap_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -emit-obj -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=OBJ-RELOCS
/// Should have two relocations against _Z4testll
// OBJ-RELOCS:      Section ({{.+}}) .rela.gcc_except_table {
// OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll
// OBJ-RELOCS-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll
// OBJ-RELOCS-NEXT: }

/// This should work with both -z text and -z notext
/// Check that .gcc_except_table ends up in the relro section and the relocations are correct
// RUN: ld.lld -shared %t.o -o %t.so -z notext
// RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s
// RUN: ld.lld -shared %t.o -o %t.so -z text
// RUN: llvm-readelf -r --section-mapping --sections --program-headers --cap-relocs  %t.so | FileCheck %s

// CHECK:      Relocation section '.rel.dyn' at offset {{.+}} contains 4 entries:
// CHECK-NEXT: Offset             Info             Type                            Symbol's Value  Symbol's Name
// CHECK-NEXT:                     R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE
// CHECK-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 000000000001{{.+}} _Z4testll
// CHECK-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 000000000001{{.+}} _Z4testll
// CHECK-NEXT:                     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __gxx_personality_v0
// CHECK-EMPTY:
// CHECK-NEXT: Relocation section '.rel.plt' at offset {{.+}} contains 3 entries:
// CHECK-NEXT: Offset             Info             Type                            Symbol's Value   Symbol's Name
// CHECK-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 _Z11external_fnl
// CHECK-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_begin_catch
// CHECK-NEXT:                R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE 0000000000000000 __cxa_end_catch
// CHECK-EMPTY:
// CHECK-LABEL: There are 10 program headers, starting at
// CHECK-EMPTY:
// CHECK-NEXT: Program Headers:
// CHECK-NEXT: Type           Offset   VirtAddr PhysAddr FileSiz  MemSiz   Flg Align
// CHECK-NEXT: PHDR           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// CHECK-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x10000
// CHECK-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R E 0x10000
// CHECK-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
// CHECK-NEXT: LOAD           0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x10000
// CHECK-NEXT: DYNAMIC        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// CHECK-NEXT: GNU_RELRO      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x1
// CHECK-NEXT: GNU_STACK      0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} RW  0x0
// CHECK-NEXT: OPTIONS        0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// CHECK-NEXT: ABIFLAGS       0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} 0x{{.+}} R   0x8
// CHECK-EMPTY:
// CHECK-NEXT: Section to Segment mapping:
// CHECK-NEXT: Segment Sections...
// CHECK-NEXT: 00
// CHECK-NEXT: 01     .MIPS.abiflags .MIPS.options .dynsym .hash .dynamic .dynstr .rel.dyn .rel.plt
// CHECK-NEXT: 02     .text
// CHECK-NEXT: 03     .gcc_except_table .eh_frame
// CHECK-NEXT: 04     .data .captable .got
// CHECK-NEXT: 05     .dynamic
// CHECK-NEXT: 06     .gcc_except_table .eh_frame
// CHECK-NEXT: 07
// CHECK-NEXT: 08     .MIPS.options
// CHECK-NEXT: 09     .MIPS.abiflags
// CHECK-NEXT: None   .bss .pdr .comment .symtab .shstrtab .strtab
// CHECK: There is no __cap_relocs section in the file.

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
