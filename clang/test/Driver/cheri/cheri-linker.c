// RUN: %cheri_clang -### -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 %s                              \
// RUN:   2>&1 | FileCheck --check-prefix=CHECK-CHERI %s
// CHECK-CHERI: -cheri-linker

// RUN: %cheri_clang -### -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 %s                              \
// RUN:   -cheri-linker                                                   \
// RUN:   2>&1 | FileCheck --check-prefix=CHECK-CHERI %s

// RUN: %cheri_clang -### -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 %s                              \
// RUN:   -no-cheri-linker                                                   \
// RUN:   2>&1 | FileCheck --check-prefix=CHECK-CHERI-NO-LINKER %s
// CHECK-CHERI-NO-LINKER-NOT: -cheri-linker
