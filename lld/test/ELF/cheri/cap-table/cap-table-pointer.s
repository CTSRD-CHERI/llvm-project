# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld --fatal-warnings -pie %t.o -o %t.exe
# RUN: llvm-readobj --cap-relocs --cap-table -dyn-relocations -t %t.exe | %cheri_FileCheck %s
# RUN: ld.lld --fatal-warnings -shared %t.o -o %t.so
# RUN: llvm-readobj --cap-relocs --cap-table -dyn-relocations -t %t.so | %cheri_FileCheck %s

.text
.global __start
__start:
  nop
  clcbi $c1, %captab20(_CHERI_CAPABILITY_TABLE_)($c26)


// CHECK:      Symbol {
// CHECK:         Name: _CHERI_CAPABILITY_TABLE_ (35)
// CHECK-NEXT:    Value: 0x20{{3F0|400}}
// CHECK-NEXT:    Size: [[#CAP_SIZE]]
// CHECK-NEXT:    Binding: Local (0x0)
// CHECK-NEXT:    Type: None (0x0)
// CHECK-NEXT:    Other [ (0x2)
// CHECK-NEXT:      STV_HIDDEN (0x2)
// CHECK-NEXT:    ]
// CHECK-NEXT:    Section: .captable (0x{{.+}})
// CHECK-NEXT:  }

// CHECK:      CHERI __cap_relocs [
// CHECK-NEXT:    0x0[[CAPTAB_ADDR:20(3f0|400)]]
// CHECK-SAME:    (_CHERI_CAPABILITY_TABLE_@CAPTABLE) Base: 0x[[CAPTAB_ADDR]] (_CHERI_CAPABILITY_TABLE_@CAPTABLE+0) Length: [[#CAP_SIZE]] Perms: Object
// CHECK-NEXT: ]
// CHECK-NEXT: CHERI .captable [
// CHECK-NEXT:   0x0      _CHERI_CAPABILITY_TABLE_@CAPTABLE
// CHECK-NEXT: ]
