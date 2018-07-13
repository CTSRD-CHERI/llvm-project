// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel %s -c -o - -cheri-cap-tls-abi=cap-equiv -### 2>&1 | FileCheck -check-prefix CAP-EQUIV %s
// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel %s -c -o - -cheri-cap-tls-abi=legacy -### 2>&1 | FileCheck -check-prefix LEGACY %s
//
// TODO: at some point change the default to be cap tls
// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel %s -c -o - -### 2>&1 | FileCheck -check-prefix LEGACY %s
// CHECK that it also works with mips triple and non purecap abi (but is ignored)
// but not with x86
// RUN: %cheri_clang -cheri-cap-table-abi=pcrel %s -c -o - -cheri-cap-tls-abi=cap-equiv -### 2>&1 | FileCheck -check-prefix NOTPURECAP %s
// RUN: %clang -cheri-cap-table-abi=pcrel -target mips64-unknown-freebsd -cheri=128 %s -c -o - -cheri-cap-tls-abi=cap-equiv -### 2>&1  | FileCheck -check-prefix NOTPURECAP %s
// but not with x86
// RUN: %clang -cheri-cap-table-abi=pcrel -target x86_64-unknown-linux-gnu %s -c -o - -cheri-cap-tls-abi=cap-equiv -### 2>&1  | FileCheck -check-prefix BADTRIPLE %s

// Passing the flag multiple times is fine
// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel %s -c -o - -cheri-cap-tls-abi=legacy -cheri-cap-tls-abi=cap-equiv -### 2>&1 | FileCheck -check-prefix CAP-EQUIV %s

// Passing the flag multiple times is fine
// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel %s -c -o - -cheri-cap-tls-abi=legacy -cheri-cap-tls-abi=cap-equiv -### 2>&1 | FileCheck -check-prefix CAP-EQUIV %s

// Passing mllvm -cheri-cap-tls-abi to clang breaks all code since clang will then assume it needs to pass abi=legacy
// RUN: not %cheri_purecap_clang -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=cap-equiv -c %s -o /dev/null 2>&1 | FileCheck %s -check-prefix BAD_MLLVM

// BAD_MLLVM: error: unsupported option '-mllvm -cheri-cap-tls-abi=cap-equiv', did you mean '-cheri-cap-tls-abi=cap-equiv'?

// CAP-EQUIV: "-mllvm" "-cheri-cap-tls-abi=cap-equiv"
// LEGACY: "-mllvm" "-cheri-cap-tls-abi=legacy"
// NOTPURECAP-NOT: "-cheri-cap-tls-abi"
// BADTRIPLE: warning: argument unused during compilation: '-cheri-cap-tls-abi=cap-equiv' [-Wunused-command-line-argument]
