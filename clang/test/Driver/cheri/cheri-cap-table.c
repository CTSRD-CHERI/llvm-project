// The default is cap table now
// RUN: %cheri_purecap_clang %s -c -o - -### 2>&1 | FileCheck -check-prefix TABLE %s
// CHECK that it also works with mips triple and non purecap abi (but is ignored)
// RUN: %cheri_clang %s -c -o - -cheri-cap-table-abi=pcrel -### 2>&1 | FileCheck -check-prefix NOTPURECAP %s
// RUN: %clang -target mips64-unknown-freebsd -cheri=128 %s -c -o - -cheri-cap-table-abi=pcrel -### 2>&1  | FileCheck -check-prefix NOTPURECAP %s
// but not with x86
// RUN: %clang -target x86_64-unknown-linux-gnu %s -c -o - -cheri-cap-table-abi=pcrel -### 2>&1  | FileCheck -check-prefix BADTRIPLE %s

// Passing the flag multiple times is fine
// RUN: %cheri_purecap_clang %s -c -o - -cheri-cap-table-abi=pcrel -### 2>&1 | FileCheck -check-prefix TABLE %s

// RUN: not %cheri_purecap_clang -mllvm -cheri-cap-table-abi=pcrel -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MLLVM
// BAD_MLLVM: error: unsupported option '-mllvm -cheri-cap-table-abi=pcrel'; did you mean '-cheri-cap-table-abi=pcrel'?

// RUN: not %cheri_purecap_clang -mllvm -mxcaptable -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-1
// BAD_MXCAPTABLE-1: error: unsupported option '-mllvm -mxcaptable'; did you mean '-mxcaptable'?
// RUN: not %cheri_purecap_clang -mllvm -mxcaptable=true -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-2
// BAD_MXCAPTABLE-2: error: unsupported option '-mllvm -mxcaptable=true'; did you mean '-mxcaptable'?
// RUN: not %cheri_purecap_clang -mllvm -mxcaptable=false -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MXCAPTABLE-3
// BAD_MXCAPTABLE-3: error: unsupported option '-mllvm -mxcaptable=false'; did you mean '-no-mxcaptable'?


// TABLE: "-mllvm" "-cheri-cap-table-abi=pcrel"
// NOTPURECAP-NOT: "-cheri-cap-table"
// BADTRIPLE: warning: argument unused during compilation: '-cheri-cap-table-abi=pcrel' [-Wunused-command-line-argument]
