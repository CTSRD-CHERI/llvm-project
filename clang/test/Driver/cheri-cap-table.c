// RUN: %cheri_clang %s -c -o - -cheri-cap-table -###
// RUN: %cheri_clang %s -c -o - -cheri-cap-table -### 2>&1 | FileCheck -check-prefix TABLE %s
// RUN: %cheri_clang %s -c -o - -no-cheri-cap-table -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// TODO: at some point change the default to be cap table
// RUN: %cheri_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix NOTABLE %s
// CHECK that it also works with mips triple
// RUN: %clang -target mips64-unknown-freebsd -cheri=128 %s -c -o - -cheri-cap-table -### 2>&1  | FileCheck -check-prefix TABLE %s
// but not with x86
// RUN: %clang -target x86_64-unknown-linux-gnu %s -c -o - -cheri-cap-table -### 2>&1  | FileCheck -check-prefix BADTRIPLE %s

// TABLE: "-mllvm" "-cheri-cap-table=true"
// NOTABLE: "-mllvm" "-cheri-cap-table=false"

// BADTRIPLE: warning: argument unused during compilation: '-cheri-cap-table' [-Wunused-command-line-argument]
