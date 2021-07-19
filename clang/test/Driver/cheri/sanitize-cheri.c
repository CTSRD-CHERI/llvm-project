/// Using -fsanitize=cheri should enable the same checks:
// RUN: %riscv64_cheri_clang %s -fsanitize=pointer-overflow -c -o - -### 2>&1 | FileCheck %s --check-prefix=POINTER-OVERFLOW --implicit-check-not=error:
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=pointer-overflow -c -o - -### 2>&1 | FileCheck %s --check-prefix=POINTER-OVERFLOW --implicit-check-not=error:
// RUN: %riscv64_cheri_clang %s -fsanitize=undefined -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNDEFINED --implicit-check-not=error:
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=undefined -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNDEFINED --implicit-check-not=error:


/// -fsanitize=cheri/-fsanitize=cheri-unrepresentable is currently only supported for purecap since enabling it for hybrid
/// would require Toolchain.cpp refactoring.
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri-unrepresentable -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNREPRESENTABLE --implicit-check-not=error:
// RUN: %riscv64_cheri_clang %s -fsanitize=cheri-unrepresentable -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-UNREPRESENTABLE --implicit-check-not=error:
// RUN: %clang -target x86-64-unknown-freebsd %s -fsanitize=cheri-unrepresentable -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-UNREPRESENTABLE --implicit-check-not=error:

// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=CHERI --implicit-check-not=error:
// RUN: %riscv64_cheri_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-CHERI --implicit-check-not=error:
// RUN: %riscv32_cheri_purecap_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=CHERI --implicit-check-not=error:
// RUN: %riscv32_cheri_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-CHERI --implicit-check-not=error:
// RUN: %cheri_purecap_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=CHERI --implicit-check-not=error:
// RUN: %cheri_clang %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-CHERI --implicit-check-not=error:
// RUN: %clang -target x86-64-unknown-freebsd %s -fsanitize=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNSUPPORTED-CHERI --implicit-check-not=error:

// POINTER-OVERFLOW: "-fsanitize=pointer-overflow" "-fsanitize-recover=pointer-overflow"
// UNREPRESENTABLE:  "-fsanitize=cheri-unrepresentable" "-fsanitize-recover=cheri-unrepresentable"
// UNDEFINED:        "-fsanitize=alignment,array-bounds,bool,builtin,enum,float-cast-overflow,integer-divide-by-zero,nonnull-attribute,null,pointer-overflow,return,returns-nonnull-attribute,shift-base,shift-exponent,signed-integer-overflow,unreachable,vla-bound,vptr"
// UNDEFINED-SAME:   "-fsanitize-recover=alignment,array-bounds,bool,builtin,enum,float-cast-overflow,integer-divide-by-zero,nonnull-attribute,null,pointer-overflow,returns-nonnull-attribute,shift-base,shift-exponent,signed-integer-overflow,vla-bound,vptr"
/// -fsanitize=cheri currently only enables cheri-unrepresentable
// CHERI:            "-fsanitize=cheri-unrepresentable" "-fsanitize-recover=cheri-unrepresentable"
// UNSUPPORTED-UNREPRESENTABLE: error: unsupported option '-fsanitize=cheri-unrepresentable' for target '{{.+}}'
// UNSUPPORTED-CHERI: error: unsupported option '-fsanitize=cheri' for target '{{.+}}'


/// Check the -fsanitize-trap flag:
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri-unrepresentable -fsanitize-trap=cheri-unrepresentable -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNREPRESENTABLE-TRAP --implicit-check-not=error:
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri -fsanitize-trap=cheri -c -o - -### 2>&1 | FileCheck %s --check-prefix=CHERI-TRAP --implicit-check-not=error:

// CHERI-TRAP:       "-fsanitize=cheri-unrepresentable" "-fsanitize-trap=cheri-unrepresentable"
// UNREPRESENTABLE-TRAP:  "-fsanitize=cheri-unrepresentable" "-fsanitize-trap=cheri-unrepresentable"

/// Check that -fsanitize=cheri works with -fsanitize-coverage=trace-pc-guard
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri-unrepresentable -fsanitize-coverage=trace-pc-guard -c -o - -### 2>&1 | FileCheck %s --check-prefix=UNREPRESENTABLE --implicit-check-not=error:
// RUN: %riscv64_cheri_purecap_clang %s -fsanitize=cheri -fsanitize-coverage=trace-pc-guard -c -o - -### 2>&1 | FileCheck %s --check-prefix=CHERI --implicit-check-not=error:
