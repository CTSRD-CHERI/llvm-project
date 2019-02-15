// RUN: %cheri_clang -c %s -### 2>&1 | FileCheck %s -check-prefixes COMMON
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -### 2>&1 | FileCheck %s -check-prefixes COMMON,OFFSET
// RUN: %cheri_clang -c %s -cheri-uintcap=addr -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR
// RUN: %cheri_clang -c %s -cheri-uintcap=addr -cheri-uintcap=offset -### 2>&1 | FileCheck %s -check-prefixes COMMON,OFFSET
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -cheri-uintcap=addr -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR

// COMMON-NOT: -cheri-uintcap=
// ADDR: "-cheri-uintcap=addr"
// OFFSET: "-cheri-uintcap=offset"
// COMMON-NOT: -cheri-uintcap=
