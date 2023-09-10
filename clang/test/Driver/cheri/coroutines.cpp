/// Coroutines are not yet supported for purecap, make sure we emit an error
/// instead crashing while trying to generate code.
/// TargetInfo::adjust()
/// See https://github.com/CTSRD-CHERI/llvm-project/issues/717
// RUN: %riscv64_cheri_purecap_clang %s -### -std=c++17 -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=PURECAP-ERR
// RUN: %riscv64_cheri_purecap_clang %s -### -std=c++20 -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=PURECAP-ERR
// PURECAP-ERR: error: unsupported option '-fcoroutines-ts' for target 'purecap riscv64-unknown-freebsd'
