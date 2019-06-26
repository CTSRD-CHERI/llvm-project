// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux-gnu %s -o %t.o
// RUN: ld.lld --export-dynamic %t.o -o %t
// BFD does not add a dynamic section when --export-dynamic is passed together
// with -Bstatic so check that we do the same
// RUN: ld.lld -Bstatic --export-dynamic %t.o -o %t-static
// RUN: llvm-readobj --dyn-syms -t -s %t | FileCheck %s
// RUN: llvm-readobj --dyn-syms -t -s %t-static | FileCheck %s -check-prefix STATIC


// Ensure that the generated binary is still a static binary and doesn't have
// the _DYNAMIC symbol or a .dynamic section
// STATIC-LABEL: Sections [
// STATIC-NOT:     Name: .dynamic
// STATIC-NOT:     Name: .interp
// STATIC:       ]
// STATIC-LABEL: Symbols [
// STATIC-NOT:     Name: _DYNAMIC
// STATIC:       ]
// Ensure that a dynamic symbol table is present when --export-dynamic
// is passed in, even when creating statically linked executables.
//
// CHECK-LABEL: DynamicSymbols [
// CHECK-NEXT:   Symbol {
// CHECK-NEXT:     Name:
// CHECK-NEXT:     Value: 0x0
// CHECK-NEXT:     Size: 0
// CHECK-NEXT:     Binding: Local
// CHECK-NEXT:     Type: None
// CHECK-NEXT:     Other: 0
// CHECK-NEXT:     Section: Undefined
// CHECK-NEXT:   }
// CHECK-NEXT:   Symbol {
// CHECK-NEXT:     Name: _start
// CHECK-NEXT:     Value: 0x201000
// CHECK-NEXT:     Size: 0
// CHECK-NEXT:     Binding: Global
// CHECK-NEXT:     Type: None
// CHECK-NEXT:     Other: 0
// CHECK-NEXT:     Section: .text
// CHECK-NEXT:   }
// CHECK-NEXT: ]

.global _start
_start:
  ret
