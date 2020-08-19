// REQUIRES: clang
// Check that the default comparison mode is inexact equals (ceq and not cexeq)
// RUN: %cheri_cc1 %S/cheri-capcmp.ll -S -o - -O | \
// RUN:   FileCheck %S/cheri-capcmp.ll --check-prefixes=CHECK-INEXACT
// RUN: %cheri_cc1 -cheri-comparison=exact %S/cheri-capcmp.ll -S -o - -O | \
// RUN:   FileCheck %S/cheri-capcmp.ll --check-prefixes=CHECK-EXACT
// RUN: %cheri_cc1 -cheri-comparison=address %S/cheri-capcmp.ll -S -o - -O | \
// RUN:   FileCheck %S/cheri-capcmp.ll --check-prefixes=CHECK-INEXACT
