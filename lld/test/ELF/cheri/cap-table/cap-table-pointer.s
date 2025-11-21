# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld --fatal-warnings -pie %t.o -o %t.exe
# RUN: llvm-readobj --cap-relocs --cap-table --dyn-relocations --symbols %t.exe | %cheri_FileCheck %s
# RUN: ld.lld --fatal-warnings -shared %t.o -o %t.so
# RUN: llvm-readobj --cap-relocs --cap-table --dyn-relocations --symbols %t.so | %cheri_FileCheck %s

.text
.global __start
__start:
  nop
  clcbi $c1, %captab20(_CHERI_CAPABILITY_TABLE_)($c26)


// CHECK:      Symbol {
// CHECK:         Name: _CHERI_CAPABILITY_TABLE_ ({{.+}})
// CHECK-NEXT:    Value: 0x[[#%X,CAPTAB_ADDR:]]
// CHECK-NEXT:    Size: [[#CAP_SIZE]]
// CHECK-NEXT:    Binding: Local (0x0)
// CHECK-NEXT:    Type: None (0x0)
// CHECK-NEXT:    Other [ (0x2)
// CHECK-NEXT:      STV_HIDDEN (0x2)
// CHECK-NEXT:    ]
// CHECK-NEXT:    Section: .captable (0x{{.+}})
// CHECK-NEXT:  }

// CHECK:      __cap_relocs {
// CHECK-NEXT:    0x[[#CAPTAB_ADDR]] DATA - 0x[[#CAPTAB_ADDR]] [0x[[#CAPTAB_ADDR]]-0x[[#%X,CAPTAB_ADDR+CAP_SIZE]]]
// CHECK-NEXT: }
// CHECK:      CHERI .captable [
// CHECK-NEXT:   0x0      _CHERI_CAPABILITY_TABLE_@CAPTABLE
// CHECK-NEXT: ]
