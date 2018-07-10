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

// Passing the flag multiple times is fine
// RUN: %cheri_purecap_clang %s -c -o - -cheri-cap-table-abi=legacy -cheri-cap-table-abi=pcrel -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: %cheri_purecap_clang %s -c -o - -no-cheri-cap-table -cheri-cap-table -no-cheri-cap-table -### 2>&1 | FileCheck -check-prefix NOTABLE %s

// Passing mllvm -cheri-cap-table-abi to clang breaks all code since clang will then assume it needs to pass abi=legacy
// RUN: not %cheri_purecap_clang -mllvm -cheri-cap-table-abi=pcrel -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MLLVM

// BAD_MLLVM: error: unsupported option '-mllvm -cheri-cap-table-abi=pcrel', did you mean '-cheri-cap-table-abi=pcrel'?

// RUN: not %cheri_purecap_clang -mllvm -mxcaptable -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-1
// BAD_MXCAPTABLE-1: error: unsupported option '-mllvm -mxcaptable', did you mean '-mxcaptable'?
// RUN: not %cheri_purecap_clang -mllvm -mxcaptable=true -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-2
// BAD_MXCAPTABLE-2: error: unsupported option '-mllvm -mxcaptable=true', did you mean '-mxcaptable'?
// RUN: not %cheri_purecap_clang -mllvm -mxcaptable=false -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-3
// BAD_MXCAPTABLE-3: error: unsupported option '-mllvm -mxcaptable=false', did you mean '-no-mxcaptable'?


// TABLE: "-mllvm" "-cheri-cap-table-abi=pcrel"
// NOTABLE: "-mllvm" "-cheri-cap-table-abi=legacy"
// NOTPURECAP-NOT: "-cheri-cap-table"
// BADTRIPLE: warning: argument unused during compilation: '-cheri-cap-table' [-Wunused-command-line-argument]
