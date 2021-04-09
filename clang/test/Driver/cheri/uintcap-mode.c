// RUN: %cheri_clang -c %s -### 2>&1 | FileCheck %s -check-prefixes COMMON
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -### 2>&1
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -### 2>&1 | FileCheck %s -check-prefixes COMMON,OFFSET
// RUN: %cheri_clang -c %s -cheri-uintcap=addr -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR
// RUN: %cheri_clang -c %s -cheri-uintcap=address -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR
// RUN: %cheri_clang -c %s -cheri-uintcap=addr -cheri-uintcap=offset -### 2>&1 | FileCheck %s -check-prefixes COMMON,OFFSET
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -cheri-uintcap=addr -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR
// RUN: %cheri_clang -c %s -cheri-uintcap=offset -cheri-uintcap=address -### 2>&1 | FileCheck %s -check-prefixes COMMON,ADDR
// RUN: not %cheri_clang -c %s -cheri-uintcap=foo -fsyntax-only 2>&1 | FileCheck %s --check-prefix=ERR
// ERR: error: invalid value 'foo' in '-cheri-uintcap=foo'

// COMMON-NOT: -cheri-uintcap=
// ADDR: "-cheri-uintcap=addr{{(ess)?}}"
// OFFSET: "-cheri-uintcap=offset"
// COMMON-NOT: -cheri-uintcap=
