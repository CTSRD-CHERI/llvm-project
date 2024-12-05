/// Coroutines are not yet supported for purecap, make sure we emit an error
/// instead crashing while trying to generate code.
/// See https://github.com/CTSRD-CHERI/llvm-project/issues/717

// RUN: %riscv64_cheri_cc1 -std=c++17 -E -dM -x c++ /dev/null \
// RUN:    | FileCheck %s --check-prefix=CXX17-NO-TS
// RUN: %riscv64_cheri_purecap_cc1 -std=c++17 -E -dM -x c++ /dev/null \
// RUN:    | FileCheck %s --check-prefix=CXX17-NO-TS
// CXX17-NO-TS:     #define __cplusplus 201703L
// CXX17-NO-TS-NOT: #define __cpp_coroutines
// CXX17-NO-TS-NOT: #define __cpp_impl_coroutine

// RUN: %riscv64_cheri_cc1 -std=c++20 -E -dM -x c++ /dev/null \
// RUN:    | FileCheck %s --check-prefixes=HYBRID-CXX20
// HYBRID-CXX20: #define __cplusplus 202002L
// HYBRID-CXX20: #define __cpp_impl_coroutine 201902L
// RUN: %riscv64_cheri_purecap_cc1 -std=c++20 -E -dM -x c++ /dev/null \
// RUN:    | FileCheck %s --check-prefix=PURECAP-CXX20
// PURECAP-CXX20: #define __cplusplus 202002L
// PURECAP-CXX20-NOT: #define __cpp_coroutines
// PURECAP-CXX20-NOT: #define __cpp_impl_coroutine
