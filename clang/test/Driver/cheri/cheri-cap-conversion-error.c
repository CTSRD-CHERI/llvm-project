// RUN: %cheri_clang -c %s -### 2>&1 -cheri-conversion-error=ignore | FileCheck %s -check-prefix=IGNORE
// RUN: %cheri_clang -c %s -### 2>&1 -cheri-conversion-error=error | FileCheck %s -check-prefix=ERROR
// RUN: %cheri_clang -c %s -### 2>&1 | FileCheck %s -check-prefix=NONE

// NONE-NOT: -cheri-conversion-error
// IGNORE: -cheri-conversion-error=ignore
// ERROR: -cheri-conversion-error=error
