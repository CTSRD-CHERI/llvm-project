// RUN: unset CHERI_CAP_TABLE
// RUN: unset ISA
// RUN: unset LLVM_BRANCH

// RUN: %cheri_purecap_clang %s -c -o - -cheri-cap-table -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: %cheri_purecap_clang %s -c -o - -no-cheri-cap-table -### 2>&1 | FileCheck -check-prefix NOTABLE %s

// TODO: at some point change the default to be cap table
// RUN: %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// CHECK that it also works with mips triple and non purecap abi (but is ignored)
// RUN: %cheri_clang %s -c -o - -cheri-cap-table -### 2>&1 | FileCheck -check-prefix NOTPURECAP %s
// RUN: %clang -target mips64-unknown-freebsd -cheri=128 %s -c -o - -cheri-cap-table -### 2>&1  | FileCheck -check-prefix NOTPURECAP %s
// but not with x86
// RUN: %clang -target x86_64-unknown-linux-gnu %s -c -o - -cheri-cap-table -### 2>&1  | FileCheck -check-prefix BADTRIPLE %s

// RUN: env LLVM_BRANCH=cap-table %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: env ISA=cap-table %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: env CHERI_CAP_TABLE=1 %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: env ISA=cap-table %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: env ISA=cap-table %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: env CHERI_CAP_TABLE=0 %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// RUN: env CHERI_CAP_TABLE=no %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// RUN: env CHERI_CAP_TABLE=false %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// RUN: env CHERI_CAP_TABLE=hello %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s

// TABLE: "-mllvm" "-cheri-cap-table-abi=pcrel"
// NOTABLE: "-mllvm" "-cheri-cap-table-abi=legacy"
// NOTPURECAP-NOT: "-cheri-cap-table"
// BADTRIPLE: warning: argument unused during compilation: '-cheri-cap-table' [-Wunused-command-line-argument]
