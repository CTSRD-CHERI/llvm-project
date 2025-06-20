// REQUIRES: riscv
/// Check that we correctly set (or don't set) st_size for section start symbols
/// and that the capability relocation has the correct size.
/// In hybrid mode we don't want to change the symbol table st_size value, but
/// the __cap_relocs should still have the correct size value.

// RUN: rm -rf %t && split-file %s %t && cd %t
// RUN: %riscv64_cheri_llvm-mc -filetype=obj test.s -o test-hybrid.o
// RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj test.s -o test-purecap.o

/// First check the default logic (no linker script)
// RUN: ld.lld --verbose test-hybrid.o -o test-hybrid.exe 2>&1 | FileCheck %s --check-prefix=LOG
// RUN: ld.lld --verbose test-purecap.o -o test-purecap.exe 2>&1 | FileCheck %s --check-prefix=LOG
// LOG-DAG: lld: Treating __start_mysection as a section start symbol
// LOG-DAG: lld: Treating __preinit_array_start as a section start symbol
// LOG-DAG: lld: Treating __init_array_start as a section start symbol
// LOG-DAG: lld: Treating __fini_array_start as a section start symbol
// RUN: llvm-readelf --cap-relocs --expand-relocs --symbols test-hybrid.exe | FileCheck %s --check-prefixes=CHECK,NOSCRIPT,HYBRID,CORRECT-SIZE
// RUN: llvm-readelf --cap-relocs --expand-relocs --symbols test-purecap.exe | FileCheck %s --check-prefixes=CHECK,NOSCRIPT,PURECAP-NOSCRIPT,CORRECT-SIZE

/// And also using a custom linker script that provides the start/stop symbols
/// FIXME: this is currently broken
// RUN: ld.lld --verbose test-hybrid.o -o test-hybrid.exe --script test.script 2>&1
// TODO: symbols are not marked as section start/stop for linker scripts
// | FileCheck %s --check-prefix=LOG
// RUN: ld.lld --verbose test-purecap.o -o test-purecap.exe --script test.script 2>&1
// TODO: symbols are not marked as section start/stop for linker scripts
// | FileCheck %s --check-prefix=LOG
// RUN: llvm-readelf --cap-relocs --expand-relocs --symbols test-hybrid.exe | FileCheck %s --check-prefixes=CHECK,SCRIPT,HYBRID,WRONG-SIZE
// RUN: llvm-readelf --cap-relocs --expand-relocs --symbols test-purecap.exe | FileCheck %s --check-prefixes=CHECK,SCRIPT,PURECAP-SCRIPT,WRONG-SIZE


//--- test.script
SECTIONS {
  .text 0x210000: { *(.text) }
  mysection : {
     PROVIDE_HIDDEN(__start_mysection = .);
     *(mysection);
     PROVIDE_HIDDEN(__stop_mysection = .);
  }
  .preinit_array : {
     PROVIDE_HIDDEN(__preinit_array_start = .);
     *(.preinit_array);
     PROVIDE_HIDDEN(__preinit_array_end = .);
  }
  .init_array : {
     PROVIDE_HIDDEN(__init_array_start = .);
     *(.init_array);
     PROVIDE_HIDDEN(__init_array_end = .);
  }
  .fini_array : {
     PROVIDE_HIDDEN(__fini_array_start = .);
     *(.fini_array);
     PROVIDE_HIDDEN(__fini_array_end = .);
  }
  .data.rel.ro : {
     *(.data.rel.ro);
  }
  __cap_relocs : {
     *(__cap_relocs);
  }
}

//--- test.s

 .section .preinit_array, "a", @preinit_array
 .balign 1024
 .8byte 0

 .section .init_array, "a", @init_array
 .balign 1024
 .8byte 0

 .section .fini_array, "a", @fini_array
 .balign 1024
 .8byte 0

 .section mysection, "a", @progbits
 .balign 1024
 .8byte 0

 .text
 .balign 1024

 .globl _start
 .type _start, @function
_start: ret

 .data.rel.ro

 .chericap __preinit_array_start
 .chericap __preinit_array_end

 .chericap __init_array_start
 .chericap __init_array_end

 .chericap __fini_array_start
 .chericap __fini_array_end

 .chericap __start_mysection

 .chericap __stop_mysection

/// In purecap mode, the start symbols should have size 8, in hybrid mode
/// all symbols should have a zero st_size for backwards compatibility.
// PURECAP-NOSCRIPT:  [[#%.16x,PREINIT_START:]]  [[#START_SYM_SIZE:8]] NOTYPE  LOCAL  HIDDEN      [[#]] __preinit_array_start
// FIXME: in the linker script case we do not set the size correctly
// PURECAP-SCRIPT:   [[#%.16x,PREINIT_START:]]  [[#START_SYM_SIZE:0]] NOTYPE  LOCAL  HIDDEN      [[#]] __preinit_array_start
// HYBRID:    [[#%.16x,PREINIT_START:]]  [[#START_SYM_SIZE:0]] NOTYPE  LOCAL  HIDDEN      [[#]] __preinit_array_start
// CHECK-DAG: [[#%.16x,PREINIT_START+8]] 0                     NOTYPE  LOCAL  HIDDEN      [[#]] __preinit_array_end
// CHECK-DAG: [[#%.16x,INIT_START:]]     [[#START_SYM_SIZE]]   NOTYPE  LOCAL  HIDDEN      [[#]] __init_array_start
// CHECK-DAG: [[#%.16x,INIT_START+8]]    0                     NOTYPE  LOCAL  HIDDEN      [[#]] __init_array_end
// CHECK-DAG: [[#%.16x,FINI_START:]]     [[#START_SYM_SIZE]]   NOTYPE  LOCAL  HIDDEN      [[#]] __fini_array_start
// CHECK-DAG: [[#%.16x,FINI_START+8]]    0                     NOTYPE  LOCAL  HIDDEN      [[#]] __fini_array_end
/// Without a linker script C-compatible sections names get a proected global symbol instead of local hidden
// NOSCRIPT-DAG: [[#%.16x,MY_START:]]    [[#START_SYM_SIZE]]   NOTYPE  GLOBAL  PROTECTED  [[#]] __start_mysection
// NOSCRIPT-DAG: [[#%.16x,MY_START+8]]   0                     NOTYPE  GLOBAL  PROTECTED  [[#]] __stop_mysection
// SCRIPT-DAG: [[#%.16x,MY_START:]]      [[#START_SYM_SIZE]]   NOTYPE  LOCAL  HIDDEN      [[#]] __start_mysection
// SCRIPT-DAG: [[#%.16x,MY_START+8]]     0                     NOTYPE  LOCAL  HIDDEN      [[#]] __stop_mysection
// CHECK: CHERI __cap_relocs [
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location: [[#%#X,FIRST_RELOC:]]
// CHECK-NEXT:     Base: __preinit_array_start ([[#%#X,PREINIT_START]])
// CHECK-NEXT:     Offset: 0
// WRONG-SIZE-NEXT:   Length: 0
// CORRECT-SIZE-NEXT: Length: 8
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location: [[#%#X,FIRST_RELOC+0x10]]
// CHECK-NEXT:     Base: __preinit_array_end ([[#%#X,PREINIT_START+8]])
// CHECK-NEXT:     Offset: 0
// CHECK-NEXT:     Length: 0
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x20]]
// CHECK-NEXT:     Base: __init_array_start ([[#%#X,INIT_START]])
// CHECK-NEXT:     Offset: 0
// WRONG-SIZE-NEXT:   Length: 0
// CORRECT-SIZE-NEXT: Length: 8
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x30]]
// CHECK-NEXT:     Base: __init_array_end ([[#%#X,INIT_START+8]])
// CHECK-NEXT:     Offset: 0
// CHECK-NEXT:     Length: 0
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x40]]
// CHECK-NEXT:     Base: __fini_array_start ([[#%#X,FINI_START]])
// CHECK-NEXT:     Offset: 0
// WRONG-SIZE-NEXT:   Length: 0
// CORRECT-SIZE-NEXT: Length: 8
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x50]]
// CHECK-NEXT:     Base: __fini_array_end ([[#%#X,FINI_START+8]])
// CHECK-NEXT:     Offset: 0
// CHECK-NEXT:     Length: 0
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x60]]
// CHECK-NEXT:     Base: __start_mysection ([[#%#X,MY_START]])
// CHECK-NEXT:     Offset: 0
// WRONG-SIZE-NEXT:   Length: 0
// CORRECT-SIZE-NEXT: Length: 8
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
// CHECK-NEXT:   }
// CHECK-NEXT:   Relocation {
// CHECK-NEXT:     Location:  [[#%#X,FIRST_RELOC+0x70]]
// CHECK-NEXT:     Base: __stop_mysection ([[#%#X,MY_START+8]])
// CHECK-NEXT:     Offset: 0
// CHECK-NEXT:     Length: 0
// CHECK-NEXT:     Permissions: Constant (0x4000000000000000)
